#ifndef  CRAILS_HTTP_CONNECTION_H
# define CRAILS_HTTP_CONNECTION_H

# include "../http.hpp"
# include <boost/asio/ip/tcp.hpp>
# include <boost/beast/core.hpp>
# include <optional>

namespace Crails
{
  class Server;

  class Connection : public std::enable_shared_from_this<Connection>
  {
  public:
    typedef std::function<void(Connection&)> RequestHandler;
    typedef boost::beast::http::request_parser<boost::beast::http::buffer_body> HttpParser;

    Connection(const Server&, boost::asio::ip::tcp::socket);
    Connection(const Server&, HttpRequest); // Only needed for test purposes
    ~Connection();

    void start();
    void write();
    void close();

    HttpParser&        get_parser() { return *parser; }
    const HttpRequest& get_request() const { return request; }
    HttpResponse&      get_response() { return response; }
    boost::beast::tcp_stream& get_stream() { return stream; }
    const std::string& get_connection_id() const { return connection_id; }

    void expires_after(std::chrono::duration<int>);

    void on_received_body_chunk(std::function<void(std::string_view)> callback) { body_chunk_callback = callback; }

    std::size_t get_content_length_remaining() const;
    void get_body(std::function<void (std::string_view)>);

    template<typename CONNECTION>
    std::shared_ptr<CONNECTION> move_to()
    {
      auto new_connection = std::shared_ptr<CONNECTION>(*this);
      return new_connection;
    }

  private:
    void expect_read();
    void expect_body();
    void read(boost::beast::error_code ec, std::size_t bytes_transferred);
    void read_header(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t);
    void on_read_error(boost::beast::error_code);
    void reset_body_chunk_callback() { body_chunk_callback = std::function<void(std::string_view)>(); }

    const Server&             server;
    boost::beast::tcp_stream  stream;
    boost::beast::flat_buffer buffer{8192};

    HttpRequest               request{};
    HttpResponse              response{};
    std::string               connection_id;
    std::optional<HttpParser> parser;
    char                      body_buffer[8192];
    unsigned int              max_body_size = 5242880;

    std::function<void(std::string_view)> body_chunk_callback;
    std::string body;
  };
}

#endif // CONNECTION_H
