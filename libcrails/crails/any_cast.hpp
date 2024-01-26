#ifndef  CRAILS_ANY_CAST_HPP
# define CRAILS_ANY_CAST_HPP

# include <any>
# include <string>
# include <map>

namespace Crails
{
  std::string any_cast(const std::any& val);

  template<typename V>
  struct AnyCaster
  {
    static V defaults_to(const std::map<std::string, std::any>& a, const std::string& k, const V def)
    {
      typename std::map<std::string, std::any>::const_iterator it = a.find(k);

      if (it == a.end())
        return def;
      return std::any_cast<V>(it->second);
    }
  };

  template<>
  struct AnyCaster<std::string>
  {
    static std::string defaults_to(const std::map<std::string, std::any>& a, const std::string& k, const std::string& def)
    {
      typename std::map<std::string, std::any>::const_iterator it = a.find(k);

      if (it == a.end())
        return def;
      return Crails::any_cast(it->second);
    }
  };

  template<typename V>
  static V defaults_to(const std::map<std::string, std::any>& a, const std::string& k, const V def)
  {
    return AnyCaster<V>::defaults_to(a, k, def);
  }
}

#endif
