#include <crails/program_options.hpp>
#include <crails/session_store/no_session_store.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include "test_server.hpp"

#undef NODEBUG
#include <cassert>

using namespace Crails;
using namespace std;

struct BaseTestError
{
  virtual const char* what() const { return "base-test-error"; }
  virtual ~BaseTestError() = default;
};

struct DerivedTestError : public BaseTestError
{
  const char* what() const override { return "derived-test-error"; }
};

struct UnrelatedTestError {};

struct AppSpecificError : public std::runtime_error
{
  AppSpecificError() : std::runtime_error("app-specific-error") {}
};

int main()
{
  // Unhandled exception type should fallback to generic handler
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw UnrelatedTestError(); };

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(handler->was_called);
  }

  // Catches exception from RequestParsers as well
  {
    auto parser  = new ThrowingParser();
    auto handler = new TestHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->return_value = true;
    parser->thrower = []() { throw std::logic_error("bad parse"); };

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(parser->was_called);
    assert(!handler->was_called); // pipeline never reaches the handler stage
  }

  // Custom handler can callback to the default handler
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw DerivedTestError(); };

    bool reported = false;

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->register_catcher<BaseTestError>([&](Crails::Context& context, const BaseTestError& e)
    {
      reported = true;
      server->call_default_handler(context, "BaseTestError", e.what());
    });
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(reported);
  }

  // Respects the registration order (FIFO style)
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw DerivedTestError(); };

    bool base_called = false, derived_called = false;

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->register_catcher<BaseTestError>([&](Crails::Context& context, const BaseTestError& e)
    {
      base_called = true;
      server->call_default_handler(context, "BaseTestError", e.what());
    });
    server->register_catcher<DerivedTestError>([&](Crails::Context& context, const DerivedTestError& e)
    {
      derived_called = true;
      server->call_default_handler(context, "DerivedTestError", e.what());
    });
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(base_called);
    assert(!derived_called);
  }

  // Same thing for the reverse scenario
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw DerivedTestError(); };

    bool base_called = false, derived_called = false;

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->register_catcher<DerivedTestError>([&](Crails::Context& context, const DerivedTestError& e)
    {
      derived_called = true;
      server->call_default_handler(context, "DerivedTestError", e.what());
    });
    server->register_catcher<BaseTestError>([&](Crails::Context& context, const BaseTestError& e)
    {
      base_called = true;
      server->call_default_handler(context, "BaseTestError", e.what());
    });
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(derived_called);
    assert(!base_called);
  }

  // Does not panic when exception handlers throws
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw UnrelatedTestError(); };

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->register_catcher<UnrelatedTestError>([](Crails::Context&, const UnrelatedTestError&)
    {
      throw std::logic_error("the exception handler itself has a bug");
    });
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
  }

  // Re-entrency from the same thread
  {
    auto parser  = new TestParser();
    auto handler = new TestHandler();
    handler->return_value = true;
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");

    bool inner_seen = false, outer_continued = false;

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->register_catcher<UnrelatedTestError>([&](Crails::Context&, const UnrelatedTestError&)
    {
      inner_seen = true;
    });
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->protect([&]()
    {
      context->protect([&]()
      {
        throw UnrelatedTestError();
      });
      outer_continued = true;
    });

    assert(inner_seen);
    assert(outer_continued);

    context->test_run();
    assert(future_status.get() == 200);
  }

  // Re-entrency from a different thread should wait for the previous task to finish
  {
    auto parser  = new TestParser();
    auto handler = new TestHandler();
    handler->return_value = true;
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);

    std::atomic<bool> main_holds_lock{false};
    std::atomic<bool> background_attempted{false};
    std::atomic<bool> background_saw_main_still_holding{false};
    std::atomic<bool> background_done{false};
    std::thread       background;

    context->protect([&]()
    {
      main_holds_lock = true;

      background = std::thread([&]()
      {
        background_attempted = true;
        context->protect([&]()
        {
          if (main_holds_lock.load())
            background_saw_main_still_holding = true;
          background_done = true;
        });
      });

      while (!background_attempted.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      main_holds_lock = false;
    });

    // We do not join from within the first task, for obvious reasons
    background.join(); 

    assert(background_done.load());
    assert(!background_saw_main_still_holding.load());

    context->test_run();
    assert(handler->was_called);
  }

  // Handlers catch std::exception inheritors before its fallback handler
  {
    auto parser  = new TestParser();
    auto handler = new ThrowingHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->thrower = []() { throw AppSpecificError(); };

    bool app_specific_handler_called = false;

    SingletonInstantiator<ExceptionTestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;

    server->register_catcher<AppSpecificError>([&](Crails::Context& context, const AppSpecificError& e)
    {
      app_specific_handler_called = true;
      server->call_default_handler(context, "AppSpecificError", e.what());
    });
    server->register_default_std_exception_catcher();

    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context    = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 500);
    assert(app_specific_handler_called);
  }

  Crails::Server::cleanup();
  return 0;
}
