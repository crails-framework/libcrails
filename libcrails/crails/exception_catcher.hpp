#ifndef  EXCEPTION_CATCHER_HPP
# define EXCEPTION_CATCHER_HPP

# include <crails/utils/backtrace.hpp>
# include <sstream>
# include <thread>
# include <functional>
# include <mutex>

namespace Crails
{
  class Context;
  class Server;

  class ExceptionCatcher
  {
    friend class Context;
    friend class Server;

    struct Context
    {
      Context(Crails::Context&, std::function<void()> callback);
      Context();

      unsigned short        iterator;
      Crails::Context*      context;
      std::thread::id       thread_id;
      std::function<void()> callback;
    };

    struct MutexLock : public std::lock_guard<std::mutex>
    {
      MutexLock(const Crails::Context&);
    };

    typedef std::function<void (Context)> Function;
    typedef std::vector<Function> Functions;
  public:
    ExceptionCatcher();

    void run(Crails::Context&, std::function<void()> callback) const;
    void run_protected(Crails::Context&, std::function<void()> callback) const;

    template<typename EXCEPTION>
    void add_exception_catcher(const std::string& exception_name)
    {
      add_exception_catcher<EXCEPTION>([this, exception_name](Crails::Context& context, const EXCEPTION e)
      {
        std::stringstream stream;

        stream << boost_ext::trace(e);
        default_exception_handler(context, exception_name, e.what(), stream.str());
      });
    }

    template<typename EXCEPTION>
    void add_exception_catcher(std::function<void (Crails::Context&, const EXCEPTION)> handler)
    {
      functions.push_back([this, handler](Context exception_context)
      {
        try
        {
          exception_context.iterator++;
          if (exception_context.iterator < functions.size())
            functions[exception_context.iterator](exception_context);
          else
            exception_context.callback();
        }
        catch (const EXCEPTION e)
        {
          const MutexLock lock(*exception_context.context);
          handler(*(exception_context.context), e);
        }
      });
    }

    void default_exception_handler(Crails::Context&, const std::string& exception_name, const std::string& message, const std::string& trace);
  private:
    void response_exception(Crails::Context&, std::string exception_name, std::string message) const;

    Functions     functions;
  };
}


#endif
