#pragma once
#include <crails/server.hpp>
#include <crails/request_handler.hpp>
#include <crails/request_parser.hpp>
#include <thread>
#include <iostream>

struct TestServer : public Crails::Server
{
  SINGLETON_IMPLEMENTATION(TestServer, Crails::Server)
public:
  TestServer()
  {
    set_environment(Crails::Test);
  }

  template<typename LISTA, typename LISTB>
  void test_setup(const LISTA& handlers, const LISTB& parsers)
  {
    for (auto* handler : handlers)
      add_request_handler(handler);
    for (auto* parser : parsers)
      add_request_parser(parser);
  }
};

struct TestParser : public Crails::RequestParser
{
  mutable bool was_called = false;
  Crails::RequestParser::Status return_value = Crails::RequestParser::Continue;

  void operator()(Crails::Context& context, std::function<void(Crails::RequestParser::Status)> callback) const override
  {
    was_called = true;
    callback(return_value);
  }
};

struct TestHandler : public Crails::RequestHandler
{
  mutable bool was_called = false;
  bool return_value = false;

  TestHandler() : Crails::RequestHandler("test")
  {
  }

  virtual void operator()(Crails::Context& context, std::function<void(bool)> callback) const override
  {
    was_called = true;
    callback(return_value);
  }
};

struct ThreadHandler : public TestHandler
{
  std::function<void()> task;
  
  void operator()(Crails::Context& context, std::function<void(bool)> callback) const override
  {
    std::thread t([this, &context, callback]()
    {
      context.protect([this, callback]()
      {
        task();
        was_called = true;
        callback(return_value);
      });
    });
    t.detach();
  }
};

struct TestContext : public Crails::Context
{
  TestContext(const TestServer& server, Crails::Connection& connection)
    : Crails::Context(server, connection)
  {
  }

  void test_run() { run(); }
};
