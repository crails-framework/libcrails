#ifndef  CRAILS_HTTP_CONNECTION_H
# define CRAILS_HTTP_CONNECTION_H

# include "../http.hpp"
# include <boost/asio/ip/tcp.hpp>
# include <boost/beast/core.hpp>

namespace Crails
{
  class Server;

  class Connection : public std::enable_shared_from_this<Connection>
  {
  public:
    typedef std::function<void(Connection&)> RequestHandler;

    Connection(const Server&, boost::asio::ip::tcp::socket);
    Connection(const Server&, HttpRequest); // Only needed for test purposes
    ~Connection();

    void start();
    void write();
    void close();

    const HttpRequest& get_request() const { return request; }
    HttpResponse&      get_response() { return response; }
    boost::beast::tcp_stream& get_stream() { return stream; }
    const std::string& get_connection_id() const { return connection_id; }

    template<typename CONNECTION>
    std::shared_ptr<CONNECTION> move_to()
    {
      auto new_connection = std::shared_ptr<CONNECTION>(*this);
      return new_connection;
    }

  private:
    void expect_read();
    void read(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t);

    const Server&             server;
    boost::beast::tcp_stream  stream;
    boost::beast::flat_buffer buffer{8192};
    HttpRequest               request{};
    HttpResponse              response{};
    std::string               connection_id;
  };
}

#endif // CONNECTION_H
