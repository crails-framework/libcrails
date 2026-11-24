#include "watchdog.hpp"
#include <crails/logger.hpp>
#include <cstdlib>
#include <string>

using namespace std;
using namespace Crails;

static const chrono::milliseconds fallback_timeout = 30s;

chrono::milliseconds Watchdog::default_timeout()
{
  const char* variable = getenv("CRAILS_WATCHDOG_TIMEOUT");
  const int seconds = variable ? atoi(variable) : 0;

  return seconds > 0 ? chrono::seconds(seconds) : fallback_timeout;
}

Watchdog::Watchdog(boost::asio::io_context& io_context, chrono::milliseconds timeout) :
  io_context(io_context), timer(io_context), timeout(timeout)
{
  beat();
}

Watchdog::~Watchdog()
{
  stop();
}

void Watchdog::beat()
{
  last_beat = Clock::now().time_since_epoch().count();
}

void Watchdog::start()
{
  beat();
  schedule_heartbeat();
  thread = std::thread([this]() { watch(); });
}

void Watchdog::stop()
{
  {
    lock_guard<std::mutex> lock(state_mutex);
    stopping = true;
  }
  wake_up.notify_all();
  if (thread.joinable())
    thread.join();
  timer.cancel();
}

void Watchdog::schedule_heartbeat()
{
  timer.expires_after(timeout / 3);
  timer.async_wait([this](const boost::system::error_code& error)
  {
    if (!error)
    {
      beat();
      schedule_heartbeat();
    }
  });
}

void Watchdog::watch()
{
  unique_lock<std::mutex> lock(state_mutex);

  while (!stopping)
  {
    const Clock::time_point deadline = Clock::time_point(Clock::duration(last_beat.load())) + timeout;

    if (wake_up.wait_until(lock, deadline, [this]() { return stopping; }))
      return ;
    if (Clock::now() >= Clock::time_point(Clock::duration(last_beat.load())) + timeout)
    {
      logger << Logger::Error
        << "!! Watchdog: the server did not respond for " << chrono::duration_cast<chrono::seconds>(timeout).count()
        << " seconds. The io threads might be stuck (or it failed to shut down)." << Logger::endl
        << "!! Watchdog: aborting to produce a core dump. Inspect it with `thread apply all bt` in gdb." << Logger::endl;
      abort();
    }
  }
}
