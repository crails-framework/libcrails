#include "environment.hpp"

using namespace std;

namespace Crails
{
  string environment_name(Environment environment)
  {
    switch (environment)
    {
    case Production:  break ;
    case Staging:     return "staging";
    case Development: return "development";
    case Test:        return "test";
    }
    return "production";
  }
}
