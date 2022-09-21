#include <crails/logger.hpp>
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
  void render_error_view(BuildingResponse& out, HttpStatus code, Params& params);
}

ExceptionCatcher::ExceptionCatcher()
{}

void ExceptionCatcher::run_protected(Crails::Context& context, std::function<void()> callback) const
{
  Context exception_context(context);

  exception_context.iterator  = 0;
  exception_context.callback  = callback;
  exception_context.thread_id = std::this_thread::get_id();
  context.exception_context   = exception_context;
  try
  {
    if (exception_context.iterator < functions.size())
      functions[exception_context.iterator](exception_context);
    else
      callback();
  }
  catch (...)
  {
    response_exception(context, "Unknown exception", "Unfortunately, no data about it was harvested");
  }
}

void ExceptionCatcher::run(Crails::Context& context, std::function<void()> callback) const
{
  if (context.exception_context.thread_id != std::this_thread::get_id())
  {
    shared_ptr<Crails::Context> shared_context = context.shared_from_this();

    run_protected(context, callback);
    if (context.exception_context.thread_id == std::this_thread::get_id())
      context.exception_context.thread_id = std::thread::id();
  }
  else
    callback();
}

void ExceptionCatcher::response_exception(Crails::Context& context, string e_name, string e_what) const
{
  logger << Logger::Error << "# Catched exception " << e_name << ": " << e_what;
  if (context.params["backtrace"].exists())
    logger << "\n" << context.params["backtrace"].as<string>();
  logger << Logger::endl;
  if (Crails::environment == Crails::Production)
    render_error_view(context.response, HttpStatus::internal_server_error, context.params);
  else
    render_exception_view(context, e_name, e_what);
  context.on_finished();
}

void ExceptionCatcher::default_exception_handler(Crails::Context& context, const string& exception_name, const string& message, const string& trace)
{
  if (trace.length() > 0)
    context.params["backtrace"] = trace;
  response_exception(context, exception_name, message);
}
