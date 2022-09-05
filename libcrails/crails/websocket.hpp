#ifndef  CRAILS_WEBSOCKET_HPP
# define CRAILS_WEBSOCKET_HPP

# include "http.hpp"
# include <boost/beast/core.hpp>
# include <boost/beast/websocket.hpp>
# include <boost/asio/buffer.hpp>
# include <list>
# include <mutex>

namespace Crails
{
  class Context;

  class WebSocket : public std::enable_shared_from_this<WebSocket>
  {
  protected:
    WebSocket(Context& connection);
  public:
    enum MessageType
    {
      TextMessage,
      BinaryMessage
    };

    typedef std::pair<std::string, MessageType> Message;

    void accept(const HttpRequest&);
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
    void write_next_message();

    boost::beast::websocket::stream<boost::beast::tcp_stream> stream;
    boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>> dynamic_buffer;
    std::string        receive_buffer;
    std::list<Message> send_buffers;
    bool               reading = false;
    bool               writing = false;
    std::mutex         write_mutex;
  };
}

#endif
