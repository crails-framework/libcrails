#ifndef  CRAILS_ANY_CAST_HPP
# define CRAILS_ANY_CAST_HPP

# include <boost/any.hpp>
# include <string>

namespace Crails
{
  std::string any_cast(const boost::any& val);

  template<typename V>
  static V defaults_to(const std::map<std::string, boost::any>& a, const std::string& k, const V def)
  {
    typename std::map<std::string, boost::any>::const_iterator it = a.find(k);

    if (it == a.end())
      return def;
    return boost::any_cast<V>(it->second);
  }
}

#endif
