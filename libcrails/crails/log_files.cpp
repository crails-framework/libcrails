#include "log_files.hpp"
#include "program_options.hpp"
#include <iostream>

using namespace std;
using namespace boost;
using namespace Crails;

LogFiles::LogFiles(int argc, const char** argv)
{
  ProgramOptions options(argc, argv);

  load_from_program_options(options);
}

LogFiles::LogFiles(const ProgramOptions& options)
{
  load_from_program_options(options);
}

LogFiles::~LogFiles()
{
  logger.set_stdout(cout);
  logger.set_stderr(cerr);
  event_log.close();
  error_log.close();
}

void LogFiles::load_from_program_options(const ProgramOptions& options)
{
  string event_file = options.get_log_file();
  string error_file = options.get_error_log_file();

  if (event_file.length() != 0)
  {
    event_log.open(event_file.c_str());
    if (event_log.is_open())
    {
      logger.set_stdout(event_log);
      if (error_file.length() == 0)
        logger.set_stderr(event_log);
    }
  }
  if (error_file.length() != 0)
  {
    error_log.open(error_file.c_str());
    if (error_log.is_open())
      logger.set_stderr(error_log);
  }
}

