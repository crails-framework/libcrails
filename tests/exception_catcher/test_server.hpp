#pragma once
#include <crails/server.hpp>
#include <crails/request_handler.hpp>
#include <crails/request_parser.hpp>

struct ExceptionTestServer : public Crails::Server
{
  SINGLETON_IMPLEMENTATION(ExceptionTestServer, Crails::Server)
public:
  ExceptionTestServer() { set_environment(Crails::Test); }

  template<typename LISTA, typename LISTB>
  void test_setup(const LISTA& handlers, const LISTB& parsers)
  {
    for (auto* handler : handlers) add_request_handler(handler);
    for (auto* parser : parsers)   add_request_parser(parser);
  }

  template<typename EXCEPTION>
  void register_catcher(std::function<void(Crails::Context&, const EXCEPTION&)> fn)
  {
    exception_catcher.add_exception_catcher<EXCEPTION>(fn);
  }

  void call_default_handler(Crails::Context& context, const std::string& name, const std::string& what)
  {
    exception_catcher.default_exception_handler(context, name, what, "");
  }

  void register_default_std_exception_catcher()
  {
    exception_catcher.add_exception_catcher<std::exception>("std::exception");
  }
};

struct ThrowingHandler : public Crails::RequestHandler
{
  mutable bool was_called = false;
  bool return_value = false;
  std::function<void()> thrower;

  ThrowingHandler() : Crails::RequestHandler("test")
  {
  }

  void operator()(Crails::Context&, std::function<void(bool)> callback) const override
  {
    was_called = true;
    thrower();
    callback(return_value);
  }
};

struct ThrowingParser : public Crails::RequestParser
{
  mutable bool was_called = false;
  std::function<void()> thrower;

  void operator()(Crails::Context&, std::function<void(Crails::RequestParser::Status)> callback) const override
  {
    was_called = true;
    thrower();
    callback(Crails::RequestParser::Continue);
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

struct TestContext : public Crails::Context
{
  TestContext(const ExceptionTestServer& server, Crails::Connection& connection)
    : Crails::Context(server, connection)
  {
  }

  void test_run() { run(); }
};
