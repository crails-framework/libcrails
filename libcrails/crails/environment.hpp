#ifndef  CRAILS_ENVIRONMENT_HPP
# define CRAILS_ENVIRONMENT_HPP

# include <string>

namespace Crails
{
  enum Environment
  {
    Production,
    Staging,
    Development,
    Test
  };

  extern Environment environment;

  std::string environment_name(Environment);
}

#endif
