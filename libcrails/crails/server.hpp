#ifndef  SERVER_HPP
# define SERVER_HPP

# include "http.hpp"
# include "file_cache.hpp"
# include "exception_catcher.hpp"
# include "environment.hpp"
# include <crails/utils/singleton.hpp>
# include <crails/datatree.hpp>

namespace Crails
{
  class Params;
  class Context;
  class RequestParser;
  class RequestHandler;
  class Connection;

  class Server
  {
    SINGLETON(Server)
    friend class Context;
  protected:
    Server();
    ~Server();
  public:
    typedef std::vector<RequestParser*>  RequestParsers;
    typedef std::vector<RequestHandler*> RequestHandlers;
    typedef std::vector<std::string>     Directories;

    struct Crash : public boost_ext::exception
    {
      Crash(const std::string& details) : details(details) {}
      const char* what(void) const throw() { return (details.c_str()); }
      std::string details;
    };

    void launch(int argc, const char** argv);
    void stop();
    void restart();

    static const RequestHandler*    get_request_handler(const std::string& name);
    static FileCache&               get_file_cache() { return file_cache; }
    static const Directories&       get_public_paths() { return public_paths; }
    static const std::string&       get_temporary_path() { return temporary_path; }
    static boost::asio::io_context& get_io_context();
    static void set_environment(Environment);

  protected:
    void add_request_handler(RequestHandler* request_handler);
    void add_request_parser(RequestParser* request_parser);

    static Directories public_paths;
    static std::string temporary_path;
    ExceptionCatcher   exception_catcher;

  private:
    static void throw_crash_segv();

    void initialize_exception_catcher();
    void initialize_pid_file(const std::string&) const;
    void do_restart(int argc, const char** argv);

    static RequestParsers  request_parsers;
    static RequestHandlers request_handlers;
    static FileCache       file_cache;
    bool                   running = false;
    bool                   marked_for_restart = false;
    bool                   shutdown_requested = false;
  };
}

#endif
