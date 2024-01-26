#include "any_cast.hpp"
#include <iostream>

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
}
