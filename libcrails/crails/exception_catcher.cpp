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

ExceptionCatcher::ExceptionCatcher()
{}

ExceptionCatcher::MutexLock::MutexLock(const Crails::Context& context)
  : lock_guard<mutex>(context.mutex)
{
}

ExceptionCatcher::Context::Context(Crails::Context& context, function<void()> callback) :
  iterator(0),
  context(&context),
  thread_id(std::this_thread::get_id()),
  callback(callback)
{
}

ExceptionCatcher::Context::Context() : context(nullptr), thread_id(std::thread::id())
{
}

void ExceptionCatcher::run_protected(Crails::Context& context, std::function<void()> callback) const
{
  Context exception_context(context, callback);

  try
  {
    if (exception_context.iterator < functions.size())
      functions[exception_context.iterator](exception_context);
    else
    {
      const MutexLock lock(context);

      context.exception_context = exception_context;
      callback();
    }
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
  context.response.set_status_code(HttpStatus::internal_server_error);
  logger << Logger::Error << "# Catched exception " << e_name << ": " << e_what;
  if (context.params["backtrace"].exists())
    logger << "\n" << context.params["backtrace"].as<string>();
  logger << Logger::endl;
  if (Crails::environment == Crails::Production)
    render_error_view(context, HttpStatus::internal_server_error);
  else if (Renderers::singleton::get() != nullptr)
    render_exception_view(context, e_name, e_what);
  context.on_finished();
}

void ExceptionCatcher::default_exception_handler(Crails::Context& context, const string& exception_name, const string& message, const string& trace)
{
  if (trace.length() > 0)
    context.params["backtrace"] = trace;
  response_exception(context, exception_name, message);
}
