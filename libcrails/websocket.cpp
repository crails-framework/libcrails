#include "websocket.hpp"
#include <crails/connection.hpp>

using namespace Crails;

WebSocket::WebSocket(Connection& connection) : stream(std::move(connection.get_stream().socket()))
{
  receive_buffer.reserve(4096);
  send_buffer.reserve(4096);
}

void WebSocket::run()
{
  boost::asio::dispatch(stream.get_executor(),
    boost::beast::bind_front_handler(&WebSocket::on_run, shared_from_this())
  );
}

void WebSocket::read()
{
  stream.async_read(buffer, boost::beast::bind_front_handler(&WebSocket::on_read, shared_from_this()));
}

void WebSocket::send(const std::string& message, MessageType type = TextMessage)
{
  send_buffer = message;
  stream.text(type == TextMessage);
  stream.binary(type == BinaryMessage);
  stream.async_write(
    boost::asio::dynamic_buffer(send_buffer).data(),
    boost::beast::bind_front_handler(&WebSocket::on_write, shared_from_this())
  );
}

void WebSocket::send(const char* message, std::size_t length, MessageType type = TextMessage)
{
  send_buffer.reserve(length);
  for (std::size_t i = 0 ; i < length ; ++i)
    send_buffer[i] = message[i];
  stream.text(type == TextMessage);
  stream.binary(type == BinaryMessage);
  stream.async_write(
    boost::asio::dynamic_buffer(send_buffer).data(),
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
  if (ec)
    return ; // error
  read();
}

void WebSocket::on_read(boost::beast::error_code ec, std::size_t)
{
  if (ec == boost::beast::websocket::error::closed)
  {
    disconnected();
    return ;
  }
  if (ec)
    return ; // error
  received(receive_buffer, stream.got_text() ? TextMessage : BinaryMessage);
}

void WebSocket::on_write(boost::beast::error_code ec, std::size_t)
{
  if (ec) // error
    return ;
  read();
}
