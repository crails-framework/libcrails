#ifndef  CRAILS_ANY_CAST_HPP
# define CRAILS_ANY_CAST_HPP

# include <any>
# include <string>
# include <map>

namespace Crails
{
  std::string any_cast(const std::any& val);
  void log_bad_any_cast(const std::map<std::string, std::any>& a, const std::string& k, const char* expected_type);

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

  template<>
  struct AnyCaster<std::string_view>
  {
    static std::string_view defaults_to(const std::map<std::string, std::any>& a, const std::string& k, const std::string_view def)
    {
      typename std::map<std::string, std::any>::const_iterator it = a.find(k);

      if (it == a.end())
        return def;
      if (typeid(const char*) == it->second.type())
        return std::string_view(std::any_cast<const char*>(it->second));
      if (typeid(std::string) == it->second.type())
      {
        const std::string* ptr = std::any_cast<std::string>(&(it->second));
        return std::string_view(ptr->c_str(), ptr->length());
      }
      return std::any_cast<std::string_view>(it->second);
    }
  };

  template<typename V>
  static V defaults_to(const std::map<std::string, std::any>& a, const std::string& k, const V def) noexcept
  {
    try
    {
      return AnyCaster<V>::defaults_to(a, k, def);
    }
    catch (std::bad_any_cast)
    {
      log_bad_any_cast(a, k, typeid(V).name());
      return def;
    }
  }
}

#endif
