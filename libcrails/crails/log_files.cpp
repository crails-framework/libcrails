#include "log_files.hpp"
#include "program_options.hpp"
#include "getenv.hpp"
#include <crails/logger.hpp>
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
}

void LogFiles::load_from_program_options(const ProgramOptions& options)
{
  set_history_size(Crails::getenv_as<unsigned short>("LOG_HISTORY_SIZE", 10));
  initialize(options.get_log_file(), options.get_error_log_file());
}
