#include "any_cast.hpp"
#include <iostream>

namespace Crails
{
  std::string any_cast(const boost::any& val)
  {
    if (typeid(const char*) == val.type())
      return std::string(boost::any_cast<const char*>(val));
    if (typeid(std::string_view) == val.type())
      return std::string(boost::any_cast<std::string_view>(val));
    return boost::any_cast<std::string>(val);
  }
}
