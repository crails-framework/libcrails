#include <crails/logger.hpp>
#include <sstream>
#include "context.hpp"
#include "request_parser.hpp"
#include "request_handler.hpp"

using namespace Crails;
using namespace std;

namespace Crails { void render_error_view(Context&, HttpStatus); }

Context::Context(const Server& server, Connection& connection) :
  server(server),
  connection(connection.shared_from_this()),
  response(connection)
{
}

Context::~Context()
{
  logger << Logger::Debug << "Crails::Context destroyed" << Logger::endl;
  response.send();
}

void Context::protect(std::function<void()> callback)
{
  server.exception_catcher.run(*this, callback);
}

void Context::run()
{
  logger << Logger::Debug << "Crails::Context: pipeline start" << Logger::endl;
  if (server.request_parsers.begin() != server.request_parsers.end())
  {
    run_parser(
      server.request_parsers.begin(),
      bind(&Context::on_parsed, this, placeholders::_1)
    );
  }
  else
    on_parsed(true);
  logger << Logger::Debug << "Crails::Context: pipeline end" << Logger::endl;
}

void Context::run_parser(Server::RequestParsers::const_iterator it, function<void(bool)> callback)
{
  protect([this, it, callback]()
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
  protect([this, it, callback]()
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
    render_error_view(*this, boost::beast::http::status::not_found);
  on_finished();
}

string Context::responding_to_string()
{
  const auto&  request = connection->get_request();
  const string destination(request.target());
  return params["method"].defaults_to<string>("GET") +
         " '" + params["uri"].defaults_to<string>(destination) + '\'';
}

static string output_request_timers(Data timers)
{
  int i = 0;
  ostringstream stream;

  stream << ' ';
  timers.each([&i, &stream](Data timer) -> bool
  {
    stream << (i++ == 0 ? "(" : ", ");
    stream << timer.get_key() << ": " << timer.as<float>() << 's';
    return true;
  });
  stream << ')';
  return stream.str();
}

void Context::on_finished()
{
  float crails_time   = timer.GetElapsedSeconds();
  unsigned short code = static_cast<unsigned short>(connection->get_response().result());

  try {
    response.send();
    end_promise.set_value(code);
    logger << Logger::Info << "# Responded to "
           << [this, crails_time, code]()
    {
      return responding_to_string() +
             " with " + to_string(code) + " in " + to_string(crails_time) + 's';
    };
    if (params["response-time"].exists())
      logger << bind(output_request_timers, params["response-time"]);
    logger << Logger::endl;
  } catch (const std::future_error& error) {
    logger << Logger::Error << "!! Responding to " << [this, &error]()
    {
      return responding_to_string() + ": Context::on_finished: " + error.what();
    } << Logger::endl;
  }
}
