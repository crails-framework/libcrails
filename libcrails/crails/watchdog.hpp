#ifndef  CRAILS_WATCHDOG_HPP
# define CRAILS_WATCHDOG_HPP

# include <atomic>
# include <chrono>
# include <condition_variable>
# include <mutex>
# include <thread>
# include <boost/asio/io_context.hpp>
# include <boost/asio/steady_timer.hpp>

namespace Crails
{
  /**
   * Probe the io_context from a separate thread to detect an unresponsive server.
   *
   * A timer on the io_context beats every timeout / 3. A thread of its own
   * checks the beats: if one is missing for `timeout`, it logs what happened
   * and calls std::abort(), which produces a core dump and lets your supervisor
   * restart the server (systemd: Restart=always, LimitCORE=infinity ;
   * FreeBSD: daemon -r, or rc's supervisor).
   *
   * Server::stop() beats one last time, so a server which takes more than
   * `timeout` to shut down is aborted too.
   *
   * Enabled by --watchdog. The timeout is 30 seconds, which can be changed
   * with the CRAILS_WATCHDOG_TIMEOUT environment variable (in seconds). Keep it
   * above the duration of your slowest request: with a single io thread, a
   * request lasting longer than the timeout is a stall.
   */
  class Watchdog
  {
  public:
    Watchdog(boost::asio::io_context&, std::chrono::milliseconds timeout);
    ~Watchdog();

    static std::chrono::milliseconds default_timeout();

    void start();
    void stop();
    void beat();

  private:
    typedef std::chrono::steady_clock Clock;

    void schedule_heartbeat();
    void watch();

    boost::asio::io_context&        io_context;
    boost::asio::steady_timer       timer;
    const std::chrono::milliseconds timeout;
    std::atomic<Clock::rep>         last_beat;
    std::mutex                      state_mutex;
    std::condition_variable         wake_up;
    bool                            stopping = false;
    std::thread                     thread;
  };
}

#endif
