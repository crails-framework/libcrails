#include <boost/asio/dispatch.hpp>
#include <sstream>
#include <limits>
#include <crails/logger.hpp>
#include "connection.hpp"
#include "../context.hpp"

using namespace Crails;
using namespace boost;

template<typename Socket> static std::string socket_description(Socket& socket) { return socket.remote_endpoint().address().to_string(); }

Connection::Connection(const Server& server_, HttpRequest request) :
  server(server_),
  stream(std::move(asio::ip::tcp::socket(server.get_io_context()))),
  request(request)
{}

Connection::Connection(const Server& server_, asio::ip::tcp::socket socket_) :
  server(server_),
  stream(std::move(socket_))
{
  static thread_local unsigned int i = 0;
  std::stringstream id_stream;

  id_stream << socket_description(stream.socket()) << '/' << std::this_thread::get_id() << '/' << ++i;
  connection_id = id_stream.str();
  logger << Logger::Debug << "Crails::Connection opened: " << connection_id << Logger::endl;
  beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(3));
}

Connection::~Connection()
{
  logger << Logger::Debug << "Crails::Connection closed: " << connection_id << Logger::endl;
}

void Connection::start()
{
  asio::dispatch(
    stream.get_executor(),
    beast::bind_front_handler(&Connection::expect_read, shared_from_this())
  );
}

void Connection::expires_after(std::chrono::duration<int> duration)
{
  stream.expires_after(duration);
}

void Connection::expect_read()
{
  parser.emplace();
  parser->body_limit(std::numeric_limits<unsigned int>::max()); // TODO make this customizable
  reset_body_chunk_callback();
  request = {};
  expires_after(std::chrono::seconds(30));
  boost::beast::http::async_read_header(
    stream, buffer, *parser,
    beast::bind_front_handler(&Connection::read_header, shared_from_this())
  );
}

void Connection::expect_body()
{
  parser->get().body().data = body_buffer;
  parser->get().body().size = sizeof(body_buffer);
  boost::beast::http::async_read(
    stream, buffer, *parser,
    beast::bind_front_handler(&Connection::read, shared_from_this())
  );
}

void Connection::read_header(boost::beast::error_code ec, std::size_t bytes_transferred)
{
  if (!ec)
  {
    auto content_length = parser->content_length();

    response.keep_alive(parser->keep_alive());
    request = parser->get();
    std::make_shared<Context>(server, *this)->run();
    body = std::string();
    if (content_length && *content_length > 0)
      expect_body();
    else
      reset_body_chunk_callback();
  }
  else
    on_read_error(ec);
}

void Connection::read(beast::error_code ec, std::size_t bytes_transferred)
{
  if (!ec || ec == beast::http::error::need_buffer)
  {
    std::string_view chunk(body_buffer, bytes_transferred);

    if (body_chunk_callback && chunk.length() > 0)
    {
      if (body.length() + chunk.length() <= max_body_size)
        body += chunk;
      body_chunk_callback(chunk);
    }
    if (get_content_length_remaining() > 0)
      expect_body();
    else if (body_chunk_callback)
    {
      body_chunk_callback(std::string_view());
      reset_body_chunk_callback();
    }
  }
  else
    on_read_error(ec);
}

void Connection::on_read_error(boost::beast::error_code ec)
{
  if (ec != boost::asio::error::eof && ec != boost::asio::error::timed_out)
    logger << Logger::Debug << "!! Crails failed to read on socket: " << ec.message() << Logger::endl;
  else if (ec == boost::asio::error::eof)
    logger << Logger::Debug << connection_id << " EOF received" << Logger::endl;
  else if (ec == boost::asio::error::timed_out)
    logger << Logger::Debug << connection_id << " Connection timed out" << Logger::endl;
  else
    logger << Logger::Debug << connection_id << " Unexpected error type received" << Logger::endl;
}

void Connection::write()
{
  beast::http::async_write(
    stream, response,
    beast::bind_front_handler(&Connection::on_write, shared_from_this(), response.keep_alive())
  );
}

void Connection::on_write(bool keep_alive, beast::error_code ec, std::size_t)
{
  logger << Logger::Debug << "Crails::Connection written on: " << connection_id << ", keep_alive: " << keep_alive << Logger::endl;
  request = {};
  response = {};
  if (!ec && keep_alive)
    expect_read();
  else
    close();
}

void Connection::close()
{
  beast::error_code ec;

  logger << Logger::Debug << "Crails::Connection close: " << connection_id << Logger::endl;
  stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
}

std::size_t Connection::get_content_length_remaining() const
{
  if (parser && parser->is_header_done())
  {
    auto remaining = parser->content_length_remaining();

    return remaining ? *remaining : 0;
  }
  return 0;
}

void Connection::get_body(std::function<void (std::string_view)> callback)
{
  auto chunk_callback = body_chunk_callback;

  if (parser && parser->is_header_done() && get_content_length_remaining() == 0)
    callback(body);
  else
  {
    body_chunk_callback = [this, chunk_callback, callback](std::string_view chunk)
    {
      if (chunk_callback)
        chunk_callback(chunk);
      if (get_content_length_remaining() == 0)
        callback(body);
    };
  }
}
