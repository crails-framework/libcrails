#ifndef  CRAILS_WEBSOCKET_HPP
# define CRAILS_WEBSOCKET_HPP

# include <boost/beast/core.hpp>
# include <boost/beast/websocket.hpp>
# include <boost/asio/buffer.hpp>

namespace Crails
{
  class Connection;

  class WebSocket : public std::enable_shared_from_this<WebSocket>
  {
  protected:
    WebSocket(Connection& connection);
  public:
    enum MessageType
    {
      TextMessage,
      BinaryMessage
    };

    void run();
    void read();
    void send(const std::string& message, MessageType type = TextMessage);
    void send(const char* message, std::size_t length, MessageType type = TextMessage);
    virtual void disconnected() const {}
    virtual void received(const std::string&, MessageType) {}

  private:
    void on_run();
    void on_accept(boost::beast::error_code ec);
    void on_read(boost::beast::error_code ec, std::size_t);
    void on_write(boost::beast::error_code ec, std::size_t);

    boost::beast::websocket::stream<boost::beast::tcp_stream> stream;
    std::string               receive_buffer;
    std::string               send_buffer;
    boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>> dynamic_buffer;
  };
}

#endif
