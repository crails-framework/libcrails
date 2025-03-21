#include "program_options.hpp"
#include <memory>
#include <thread>
#include <iostream>

using namespace std;
using namespace boost;
using namespace Crails;

ProgramOptions::ProgramOptions(int argc, const char** argv, int options)
{
  program_options::options_description desc("Options");

  desc.add_options()
    ("port,p",     program_options::value<unsigned short>(), "listened port")
    ("hostname,h", program_options::value<std::string>(),    "listened host")
    ("threads,t",  program_options::value<unsigned short>(), "amount of threads")
    ("pidfile",    program_options::value<std::string>(),    "pid file")
    ("log,l",      program_options::value<std::string>(),    "log output")
    ("errors,e",   program_options::value<std::string>(),    "error log output")
    ("help", "")
    ;
  if ((options & AllowUnregistered) > 0)
    program_options::store(program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  else
    program_options::store(program_options::parse_command_line(argc, argv, desc), vm);
  program_options::notify(vm);
  if (vm.count("help"))
  {
    cout << desc << endl;
    exit(0);
  }
}

boost::asio::ip::tcp::endpoint ProgramOptions::get_endpoint() const
{
  return boost::asio::ip::tcp::endpoint{
    boost::asio::ip::make_address(get_value("hostname", std::string("0.0.0.0"))),
    get_value<unsigned short>("port", CRAILS_DEFAULT_PORT)
  };
}

unsigned short ProgramOptions::get_thread_count() const
{
  return get_value("threads", (unsigned short)(std::thread::hardware_concurrency()));
}

std::string ProgramOptions::get_pidfile_path() const
{
  return get_value("pidfile", std::string("crails.pid"));
}
