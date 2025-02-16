#include "any_cast.hpp"
#include <crails/logger.hpp>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace Crails
{
  std::string any_cast(const std::any& val)
  {
    if (typeid(const char*) == val.type())
      return std::string(std::any_cast<const char*>(val));
    if (typeid(std::string_view) == val.type())
      return std::string(std::any_cast<std::string_view>(val));
    return std::any_cast<std::string>(val);
  }

  void log_bad_any_cast(const std::map<std::string, std::any>& a, const std::string& k, const char* expected_type)
  {
    std::map<std::string, std::any>::const_iterator it = a.find(k);

    if (it != a.end())
    {
      std::any value = it->second;
#ifdef __GNUG__
      int status;
      const char* expected = abi::__cxa_demangle(expected_type, 0, 0, &status);
      const char* provided = abi::__cxa_demangle(value.type().name(), 0, 0, &status);
#else
      const char* expected = expected_type;
      const char* provided = value.type().name();
#endif

      logger << Logger::Error << "bad any_cast detected when casting variable `" << k << '`'
             << ". Excepted `" << expected << "`, got `" << provided << "`."
             << Logger::endl;
    }
  }
}
