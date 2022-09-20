#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>
#include "server/listener.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "program_options.hpp"
#include "log_files.hpp"
#include "request_parser.hpp"
#include "request_handler.hpp"
#ifdef USE_SEGVCATCH
# include <segvcatch.h>
# define initialize_segvcatch(listener) segvcatch::init_segv(listener)
#else
# define initialize_segvcatch(listener)
#endif

using namespace std;
using namespace Crails;

Crails::FileCache       Server::file_cache;
Server::RequestParsers  Server::request_parsers;
Server::RequestHandlers Server::request_handlers;

static string initialize_public_path()
{
  try
  {
    return boost::filesystem::canonical(
      boost::filesystem::current_path().string() + "/public"
    ).string();
  } catch (...) { }
  return boost::filesystem::current_path().string();
}

const string Server::public_path = initialize_public_path();

Server::Server(unsigned short thread_count)
{
  logger << ">> Pool Thread Size: " << thread_count << Logger::endl;
  initialize_exception_catcher();
  initialize_request_pipe();
}

Server::~Server()
{
  for_each(request_handlers.begin(), request_handlers.end(), [](RequestHandler* request_handler)
  {
    delete request_handler;
  });
  for_each(request_parsers.begin(), request_parsers.end(), [](RequestParser* request_parser)
  {
    delete request_parser;
  });
}

boost::asio::io_context& Server::get_io_context()
{
  static boost::asio::io_context io_context;

  return io_context;
}

void Server::launch(int argc, const char **argv)
{
  typedef SingletonInstantiator<LogFiles, const ProgramOptions&> LogFilesInstance;
  logger << Logger::Info << "## Launching the amazing Crails Server ##" << Logger::endl;
  const ProgramOptions     options(argc, argv);
  LogFilesInstance         log_files(options);
  Server                   server(options.get_thread_count());
  auto                     listener = make_shared<Listener>(server);
  boost::beast::error_code error;

  if (listener->listen(options.get_endpoint(), error))
  {
    boost::asio::io_context& io_context = get_io_context();
    boost::asio::signal_set stop_signals  (io_context, SIGINT, SIGTERM);
    boost::asio::signal_set restart_signal(io_context, SIGUSR2);
    server.initialize_pid_file(options.get_pidfile_path());
    listener->run();
    initialize_segvcatch(&CrailsServer::throw_crash_segv);
    logger << Logger::Info << "Listening to " << options.get_endpoint().address() << ':' << options.get_endpoint().port() << Logger::endl;
    stop_signals  .async_wait(std::bind(&Server::stop, &server));
    restart_signal.async_wait(std::bind(&Server::restart, &server));
    io_context.run();
    listener->stop();
    if (server.marked_for_restart)
      server.do_restart(argc, argv);
  }
  else
    logger << Logger::Error << "!! Could not listen on endpoint: " << error.message() << Logger::endl;
}

void Server::stop()
{
  boost::string_view action = marked_for_restart ? "restart" : "shut down";

  logger << Logger::Info << ">> Crails server will " << action << '.' << Logger::endl;
  logger << ">> Waiting for requests to end..." << Logger::endl;
  get_io_context().stop();
  logger << ">> Done." << Logger::endl;
}

void Server::restart()
{
  marked_for_restart = true;
  stop();
}

void Server::initialize_pid_file(const string& filepath) const
{
  ofstream output(filepath.c_str());

  if (output.is_open())
  {
    pid_t pid = getpid();

    logger << ">> PID " << pid << " (stored in " << filepath << ')' << Logger::endl;
    output << pid;
    output.close();
  }
  else
    logger << "!! Failed to open PID file " << filepath << Logger::endl;
}

void Server::initialize_exception_catcher()
{
  exception_catcher.add_exception_catcher<std::exception&>("std::exception");
}

void Server::add_request_handler(RequestHandler* request_handler)
{
  request_handlers.push_back(request_handler);
}

void Server::add_request_parser(RequestParser* request_parser)
{
  request_parsers.push_back(request_parser);
}

RequestHandler* Server::get_request_handler(const string& name)
{
  auto compare = [name](const RequestHandler* handler) { return handler->get_name() == name; };
  auto it = std::find_if(request_handlers.cbegin(), request_handlers.cend(), compare);

  return it != request_handlers.cend() ? *it : nullptr;
}

void Server::throw_crash_segv()
{
  throw Server::Crash("Segmentation Fault");
}
