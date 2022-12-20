#ifndef  CRAILS_LOG_FILES_HPP
# define CRAILS_LOG_FILES_HPP

# include <crails/log_rotate.hpp>
# include <crails/utils/singleton.hpp>
# include <fstream>

namespace Crails
{
  class ProgramOptions;

  class LogFiles : public LogRotate
  {
    SINGLETON(LogFiles)
  public:
    LogFiles(int argc, const char** argv);
    LogFiles(const ProgramOptions&);
    ~LogFiles() override;

  private:
    void load_from_program_options(const ProgramOptions&);
  };
}

#endif
