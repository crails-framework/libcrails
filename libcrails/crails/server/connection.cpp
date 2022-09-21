#include <boost/asio/dispatch.hpp>
#include <sstream>
#include "connection.hpp"
#include <crails/logger.hpp>
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
  logger << Logger::Info << "Crails::Connection opened: " << connection_id << Logger::endl;
  beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(3));
}

Connection::~Connection()
{
  logger << Logger::Info << "Crails::Connection closed: " << connection_id << Logger::endl;
}

void Connection::start()
{
  asio::dispatch(
    stream.get_executor(),
    beast::bind_front_handler(&Connection::expect_read, shared_from_this())
  );
}

void Connection::expect_read()
{
  request = {};
  stream.expires_after(std::chrono::seconds(30));
  beast::http::async_read(
    stream, buffer, request,
    beast::bind_front_handler(&Connection::read, shared_from_this())
  );
}

void Connection::read(beast::error_code ec, std::size_t bytes_transferred)
{
  if (!ec)
    std::make_shared<Context>(server, *this)->run();
  else if (ec != boost::asio::error::eof && ec != boost::asio::error::timed_out)
    logger << Logger::Debug << "!! Crails failed to read on socket: " << ec.message() << Logger::endl;
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

  stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
}
