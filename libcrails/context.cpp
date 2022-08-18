#include "context.hpp"
#include "request_parser.hpp"
#include "request_handler.hpp"
#include "logger.hpp"

using namespace Crails;
using namespace std;

namespace Crails { void render_error_view(BuildingResponse&, HttpStatus, Params&); }

Context::Context(const Server& server, Connection& connection) :
  server(server),
  connection(connection.shared_from_this()),
  response(connection)
{
}

Context::~Context()
{
  logger << Logger::Info << "Context DESTROYED" << Logger::endl;
}

void Context::run()
{
  logger << Logger::Info << "Context::run BEGIN" << Logger::endl;
  run_parser(
    server.request_parsers.begin(),
    bind(&Context::on_parsed, this, placeholders::_1)
  );
  logger << Logger::Info << "Context::run END" << Logger::endl;
}

void Context::run_parser(Server::RequestParsers::const_iterator it, function<void(bool)> callback)
{
  server.exception_catcher.run(*this, [this, it, callback]()
  {
    if (it == server.request_parsers.end())
      callback(server.request_parsers.size() > 0);
    else
    {
      auto self = shared_from_this();

      (**it)(*this, [self, it, callback](RequestParser::Status status)
      {
        if (status == RequestParser::Abort)
          callback(false);
        else if (status == RequestParser::Stop)
          callback(true);
        else
          self->run_parser(it + 1, callback);
      });
    }
  });
}

void Context::on_parsed(bool parsed)
{
  if (response.sent())
    on_handled(true);
  else if (parsed)
  {
    auto self = shared_from_this();

    run_handler(server.request_handlers.begin(), [self](bool handled)
    {
      self->on_handled(handled);
    });
  }
  else
  {
    logger << Logger::Info << "# Request could not be parsed" << Logger::endl;
    response.set_status_code(HttpStatus::bad_request);
    on_handled(false);
  }
}

void Context::run_handler(Server::RequestHandlers::const_iterator it, std::function<void(bool)> callback)
{
  server.exception_catcher.run(*this, [this, it, callback]()
  {
    if (it == server.request_handlers.end())
      callback(false);
    else
    {
      auto self = shared_from_this();

      (**it)(*this, [self, it, callback](bool request_handled)
      {
        if (request_handled)
          callback(true);
        else
          self->run_handler(it + 1, callback);
      });
    }
  });
}

void Context::on_handled(bool handled)
{
  this->handled = handled;
  if (!handled)
    render_error_view(response, boost::beast::http::status::not_found, params);
  on_finished();
}

static void output_request_timers(Data timers)
{
  int i = 0;

  logger << ' ';
  timers.each([&i](Data timer) -> bool
  {
    logger << (i++ == 0 ? "(" : ", ");
    logger << timer.get_key() << ": " << timer.as<float>() << 's';
    return true;
  });
  logger << ')';
}

void Context::on_finished()
{
  float crails_time   = timer.GetElapsedSeconds();
  unsigned short code = static_cast<unsigned short>(connection->get_response().result());

  response.send();
  logger << Logger::Info << "# Responded to "
         << params["method"].defaults_to<string>("GET")
         << " '" << params["uri"].defaults_to<string>("")
         << "' with " << code << " in " << crails_time << 's';
  if (params["response-time"].exists())
    output_request_timers(params["response-time"]);
  logger << Logger::endl;
}
