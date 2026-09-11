#include <crails/logger.hpp>
#include <crails/renderer.hpp>
#include "exception_catcher.hpp"
#include "context.hpp"
#include "server.hpp"
#include "params.hpp"
#include "environment.hpp"

using namespace std;
using namespace Crails;

namespace Crails
{
  void render_exception_view(Context& context, string& exception_name, string& exception_message);
  void render_error_view(Context& context, HttpStatus code);
}

void ExceptionCatcher::check_not_sealed() const
{
  if (sealed)
    throw boost_ext::runtime_error("ExceptionCatcher::add_exception_catcher called after Server::launch()");
}

void ExceptionCatcher::run(Crails::Context& context, function<void()> callback) const
{
  lock_guard<recursive_mutex> lock(context.mutex);

  try
  {
    callback();
  }
  catch (...)
  {
    try
    {
      dispatch_exception(context, std::current_exception());
    }
    catch (...)
    {
      try
      {
        response_exception(context, "Unknown exception", "An exception handler threw while processing this exception");
      }
      catch (...)
      {
        logger << Logger::Error << "!! ExceptionCatcher: fallback error response itself failed" << Logger::endl;
      }
    }
  }
}

void ExceptionCatcher::dispatch_exception(Crails::Context& context, std::exception_ptr eptr) const
{
  for (const Handler& handler : handlers)
  {
    if (handler(context, eptr))
      return ;
  }
  response_exception(context, "Unknown exception", "No handler was registered for this exception type");
}

void ExceptionCatcher::response_exception(Crails::Context& context, string e_name, string e_what) const
{
  context.response.set_status_code(HttpStatus::internal_server_error);
  logger << Logger::Error << "# Catched exception " << e_name << ": " << e_what;
  if (context.params["backtrace"].exists())
    logger << "\n" << context.params["backtrace"].as<string>();
  logger << Logger::endl;
  if (!context.is_finished()) [[likely]]
  {
    if (Crails::environment == Crails::Production)
      render_error_view(context, HttpStatus::internal_server_error);
    else if (Renderers::singleton::get() != nullptr)
      render_exception_view(context, e_name, e_what);
    context.on_finished();
  }
}

void ExceptionCatcher::default_exception_handler(Crails::Context& context, const string& exception_name, const string& message, const string& trace) const
{
  if (trace.length() > 0)
    context.params["backtrace"] = trace;
  response_exception(context, exception_name, message);
}
