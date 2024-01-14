#ifndef  PROGRAM_OPTIONS_HPP 
# define PROGRAM_OPTIONS_HPP

# include <boost/program_options.hpp>
# include <boost/asio/ip/tcp.hpp>

namespace Crails
{
  class ProgramOptions
  {
  public:
    enum Options
    {
      AllowUnregistered = 1
    };

    ProgramOptions(int argc, const char** argv, int options = 0);

    boost::asio::ip::tcp::endpoint get_endpoint() const;
    unsigned short                 get_thread_count() const;
    std::string                    get_pidfile_path() const;
    std::string                    get_log_file() const { return get_value<std::string>("log", ""); }
    std::string                    get_error_log_file() const { return get_value<std::string>("errors", ""); }

  private:
    template<typename T>
    T get_value(const std::string& option_name, const T& default_value) const
    {
      if (vm.count(option_name))
        return vm[option_name].as<T>();
      return default_value;
    }
 
    boost::program_options::variables_map vm;
  };
}

#endif
