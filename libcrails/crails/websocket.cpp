#include <boost/asio/dispatch.hpp>
#include "websocket.hpp"
#include "context.hpp"
#include "logger.hpp"

using namespace Crails;

WebSocket::WebSocket(Context& context) : stream(std::move(context.connection->get_stream().socket())),
  dynamic_buffer(boost::asio::dynamic_buffer(receive_buffer))
{
  receive_buffer.reserve(4096);
  send_buffer.reserve(4096);
}

void WebSocket::accept(const HttpRequest& request)
{
  stream.accept(request);
}

void WebSocket::read()
{
  stream.async_read(
    dynamic_buffer,
    boost::beast::bind_front_handler(&WebSocket::on_read, shared_from_this())
  );
}

void WebSocket::send(const std::string& message, MessageType type)
{
  send_buffer = message;
  stream.text(type == TextMessage);
  stream.binary(type == BinaryMessage);
  stream.async_write(
    boost::asio::dynamic_buffer(send_buffer).data(),
    boost::beast::bind_front_handler(&WebSocket::on_write, shared_from_this())
  );
}

void WebSocket::send(const char* message, std::size_t length, MessageType type)
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
  if (ec == boost::beast::websocket::error::closed)
  {
    logger << Logger::Info << "websocket connection closed" << Logger::endl;
    disconnected();
    return ;
  }
  if (ec)
  {
    logger << Logger::Error << "websocket read error: " << ec.message() << Logger::endl;
    return ; // error
  }
  received(receive_buffer, stream.got_text() ? TextMessage : BinaryMessage);
}

void WebSocket::on_write(boost::beast::error_code ec, std::size_t)
{
  if (ec)
  {
    logger << Logger::Error << "websocket write error: " << ec.message() << Logger::endl;
    return ;
  }
  read();
}
