#include <crails/program_options.hpp>
#include <crails/environment.hpp>
#include <iostream>
#include "test_server.hpp"
#include <crails/session_store/no_session_store.hpp>

namespace Crails
{
  Environment environment = Test;
}

#undef NODEBUG
#include <cassert>


int main()
{
  using namespace Crails;
  using namespace std;

  // Simple test
  {
    auto* parser = new TestParser();
    auto* handler = new TestHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");
    handler->return_value = true;

    SingletonInstantiator<TestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context = make_shared<TestContext>(*server, *connection);

    context->test_run();
    assert(parser->was_called);
    assert(handler->was_called);
  }
  

  // Async, stops calling handlers after a handler responds with `true`
  {
    auto parser = new TestParser();
    auto handlers = vector<ThreadHandler*>{
      new ThreadHandler(),
      new ThreadHandler(),
      new ThreadHandler(),
      new ThreadHandler(),
      new ThreadHandler(),
      new ThreadHandler()
    };
    for (auto* handler : handlers)
    {
      handler->task = []() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); };
      handler->return_value = false;
    }
    handlers[4]->return_value = true;
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");

    SingletonInstantiator<TestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(handlers, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    context->test_run();
    assert(future_status.get() == 200);
    assert(parser->was_called);
    assert(handlers[0]->was_called);
    assert(handlers[1]->was_called);
    assert(handlers[2]->was_called);
    assert(handlers[3]->was_called);
    assert(handlers[4]->was_called);
    assert(!handlers[5]->was_called);
  }

  // Catches exceptions from a thread
  {
    auto parser = new TestParser();
    auto handler = new ThreadHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");

    SingletonInstantiator<TestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    handler->task = []()
    {
      throw std::runtime_error("whops");
    };

    context->test_run();
    assert(future_status.get() == 500);
  }

  // Returns 404 for unhandled requests
  {
    auto parser = new TestParser();
    auto handler = new ThreadHandler();
    HttpRequest request;
    request.method(HttpVerb::get);
    request.target("/path");

    SingletonInstantiator<TestServer> server;
    SingletonInstantiator<Crails::NoSessionStore::Factory> store;
    server->test_setup(vector<Crails::RequestHandler*>{handler}, vector<Crails::RequestParser*>{parser});
    auto connection = make_shared<Connection>(*server, request);
    auto context = make_shared<TestContext>(*server, *connection);
    auto future_status = context->get_future();

    handler->task = []()
    {
    };

    context->test_run();
    assert(future_status.get() == 404);
  }

  return 0;
}
