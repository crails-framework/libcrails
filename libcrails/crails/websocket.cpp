#include <boost/asio/dispatch.hpp>
#include "websocket.hpp"
#include "context.hpp"
#include <crails/logger.hpp>

using namespace Crails;

WebSocket::WebSocket(Context& context) : stream(std::move(context.connection->get_stream().socket())),
  dynamic_buffer(boost::asio::dynamic_buffer(receive_buffer))
{
  receive_buffer.reserve(4096);
}

WebSocket::~WebSocket()
{
  if (!closed)
    stream.close(boost::beast::websocket::normal);
}

void WebSocket::accept(const HttpRequest& request)
{
  stream.accept(request);
}

void WebSocket::read()
{
  if (!reading)
  {
    reading = true;
    stream.async_read(
      dynamic_buffer,
      boost::beast::bind_front_handler(&WebSocket::on_read, shared_from_this())
    );
  }
}

void WebSocket::send(const std::string& message, MessageType type)
{
  std::lock_guard guard(write_mutex);

  send_buffers.push_back({message, type});
  if (!writing)
    write_next_message();
}

void WebSocket::send(const char* message, std::size_t length, MessageType type)
{
  send(std::string(message, length), type);
}

void WebSocket::write_next_message() // should be called with write_mutex locked
{
  std::list<Message>::iterator it = send_buffers.begin();

  writing = true;
  stream.text(it->second == TextMessage);
  stream.binary(it->second == BinaryMessage);
  stream.async_write(
    boost::asio::dynamic_buffer(it->first).data(),
    boost::beast::bind_front_handler(&WebSocket::on_write, shared_from_this())
  );
}

void WebSocket::on_run()
{
  using namespace boost::beast;
  stream.set_option(websocket::stream_base::timeout::suggested(role_type::server));
  stream.set_option(websocket::stream_base::decorator([](websocket::response_type& response)
  {
    response.set(Crails::HttpHeader::server, "crails-websocket");
  }));
  stream.async_accept(bind_front_handler(&WebSocket::on_accept, shared_from_this()));
}

void WebSocket::on_accept(boost::beast::error_code ec)
{
  if (!ec)
  {
    logger << Logger::Info << "websocket connection opened" << Logger::endl;
    read();
  }
  else
    logger << Logger::Error << "Could not accept websocket: " << ec.message() << Logger::endl;
}

void WebSocket::on_read(boost::beast::error_code ec, std::size_t)
{
  reading = false;
  if (!ec)
  {
    received(receive_buffer, stream.got_text() ? TextMessage : BinaryMessage);
    receive_buffer.resize(0);
  }
  else
  {
    closed = true;
    if (ec == boost::beast::websocket::error::closed)
      logger << Logger::Info << "websocket connection closed" << Logger::endl;
    else
      logger << Logger::Error << "websocket read error: " << ec.message() << Logger::endl;
    disconnected();
  }
}

void WebSocket::on_write(boost::beast::error_code ec, std::size_t)
{
  if (ec)
  {
    logger << Logger::Error << "websocket write error: " << ec.message() << Logger::endl;
    closed = true;
    disconnected();
  }
  else
  {
    std::lock_guard guard(write_mutex);

    writing = false;
    send_buffers.erase(send_buffers.begin());
    if (send_buffers.size())
      write_next_message();
    else
      read();
  }
}
