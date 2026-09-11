#ifndef  EXCEPTION_CATCHER_HPP
# define EXCEPTION_CATCHER_HPP

# include <crails/utils/backtrace.hpp>
# include <functional>
# include <sstream>
# include <string>
# include <vector>
# include <exception>

namespace Crails
{
  class Context;
  class Server;

  class ExceptionCatcher
  {
    friend class Server;
  public:
    ExceptionCatcher() = default;

    void run(Crails::Context& context, std::function<void()> callback) const;

    template<typename EXCEPTION>
    void add_exception_catcher(const std::string& exception_name)
    {
      add_exception_catcher<EXCEPTION>(
        [this, exception_name](Crails::Context& context, const EXCEPTION& e)
        {
          std::stringstream stream;

          stream << boost_ext::trace(e);
          default_exception_handler(context, exception_name, e.what(), stream.str());
        });
    }

    template<typename EXCEPTION>
    void add_exception_catcher(std::function<void (Crails::Context&, const EXCEPTION&)> handler)
    {
      check_not_sealed();
      handlers.push_back([handler](Crails::Context& context, std::exception_ptr eptr) -> bool
      {
        try
        {
          std::rethrow_exception(eptr);
        }
        catch (const EXCEPTION& e)
        {
          handler(context, e);
          return true;
        }
        catch (...)
        {
        }
        return false;
      });
    }

    void default_exception_handler(Crails::Context&, const std::string& exception_name, const std::string& message, const std::string& trace) const;

  private:
    typedef std::function<bool (Crails::Context&, std::exception_ptr)> Handler;

    void dispatch_exception(Crails::Context&, std::exception_ptr) const;
    void response_exception(Crails::Context&, std::string exception_name, std::string message) const;
    void check_not_sealed() const;
    void seal() { sealed = true; }

    std::vector<Handler> handlers;
    bool                  sealed = false;
  };
}

#endif
