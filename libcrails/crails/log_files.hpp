#ifndef  CRAILS_LOG_FILES_HPP
# define CRAILS_LOG_FILES_HPP

# include <crails/logger.hpp>
# include <crails/utils/singleton.hpp>
# include <fstream>

namespace Crails
{
  class ProgramOptions;

  class LogFiles
  {
    SINGLETON(LogFiles)
  public:
    LogFiles(int argc, const char** argv);
    LogFiles(const ProgramOptions&);
    ~LogFiles();

  private:
    void load_from_program_options(const ProgramOptions&);

    std::ofstream event_log;
    std::ofstream error_log;
  };
}

#endif
