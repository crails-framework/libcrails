#include "server.hpp"
#include <crails/logger.hpp>
#include <unistd.h>

using namespace Crails;

static char** duplicate_arguments(int argc, const char** argv)
{
  char** arguments = new char*[argc + 1];

  for (unsigned short i = 0 ; i < argc ; ++i)
  {
    size_t length = strlen(argv[i]);

    arguments[i] = new char[length];
    strncpy(arguments[i], argv[i], length);
    arguments[i][length] = 0;
  }
  arguments[argc] = 0;
  return arguments;
}

void Server::do_restart(int argc, const char** original_argv)
{
  char** argv = duplicate_arguments(argc, original_argv);

  execve(argv[0], argv, nullptr);
  logger << Logger::Error << "!! execve failed, the server won't restart" << Logger::endl;
  for (int i = 0 ; argv[i] ; ++i) delete[] argv[i];
  delete[] argv;
}
