#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <thread>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <boost/asio/post.hpp>
#include <crails/watchdog.hpp>

using namespace Crails;
using namespace std::chrono_literals;

// Runs `body` in a child process and returns its wait status.
template<typename F>
static int in_child(F body)
{
  pid_t pid = fork();
  if (pid == 0)
  {
    struct rlimit no_core = {0, 0};
    setrlimit(RLIMIT_CORE, &no_core);
    body();
    _exit(0);
  }
  int status = 0;
  waitpid(pid, &status, 0);
  return status;
}

int main()
{
  // A responsive server is left alone, and stopping the watchdog is clean.
  int status = in_child([]()
  {
    boost::asio::io_context io;
    Watchdog watchdog(io, 300ms);
    std::thread io_thread([&io]() { io.run(); });
    watchdog.start();
    std::this_thread::sleep_for(1500ms);        // 5 timeouts
    watchdog.stop();
    io.stop();
    io_thread.join();
  });
  assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);

  // All the io threads are stuck: the process aborts.
  status = in_child([]()
  {
    boost::asio::io_context io;
    Watchdog watchdog(io, 300ms);
    std::thread io_thread([&io]() { io.run(); });
    watchdog.start();
    std::this_thread::sleep_for(500ms);
    // A "request" blocks the only io thread forever.
    boost::asio::post(io, []() { sleep(60); });
    sleep(10);                                   // would be killed by the watchdog long before
    _exit(0);
  });
  assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);

  // A server which does not shut down in time is aborted too.
  status = in_child([]()
  {
    boost::asio::io_context io;
    Watchdog watchdog(io, 300ms);
    watchdog.start();
    watchdog.beat();                             // what Server::stop() does, but nobody runs the io_context
    sleep(10);
    _exit(0);
  });
  assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);

  // CRAILS_WATCHDOG_TIMEOUT overrides the default timeout.
  assert(Watchdog::default_timeout() == 30s);
  setenv("CRAILS_WATCHDOG_TIMEOUT", "5", 1);
  assert(Watchdog::default_timeout() == 5s);
  setenv("CRAILS_WATCHDOG_TIMEOUT", "garbage", 1);
  assert(Watchdog::default_timeout() == 30s);
  return 0;
}
