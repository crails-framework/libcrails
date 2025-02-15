#include <crails/program_options.hpp>
#include <crails/environment.hpp>
#include <iostream>
#include <thread>

namespace Crails
{
  Environment environment = Production;
}

#undef NODEBUG
#include <cassert>

int main()
{
  using namespace Crails;
  using namespace std;

  {
    const char* argv[] = {"server"};
    ProgramOptions subject(1, argv);
    auto endpoint = subject.get_endpoint();

    assert(endpoint.port() == CRAILS_DEFAULT_PORT);
    assert(endpoint.address().to_string() == "0.0.0.0");   
    assert(subject.get_thread_count() == thread::hardware_concurrency());
  }

  {
    const char* argv[] = {"server", "-p", "80", "-h", "127.0.0.1", "--pidfile", "/tmp/crails.pid"};
    ProgramOptions subject(7, argv);
    auto endpoint = subject.get_endpoint();

    assert(endpoint.port() == 80);
    assert(endpoint.address().to_string() == "127.0.0.1");   
    assert(subject.get_pidfile_path() == "/tmp/crails.pid");
  }

  // Rejects unregistered options
  {
    try 
    {
      const char* argv[] = {"server", "--unknown"};
      ProgramOptions subject(2, argv);
      assert(false);
    }
    catch (boost::wrapexcept<boost::program_options::unknown_option>)
    {
    }
  }

  // Allows unregistered options
  {
    try 
    {
      const char* argv[] = {"server", "--unknown"};
      ProgramOptions subject(2, argv, ProgramOptions::AllowUnregistered);
      assert(true);
    }
    catch (boost::wrapexcept<boost::program_options::unknown_option>)
    {
      assert(false);
    }
  }

  return 0;
}
