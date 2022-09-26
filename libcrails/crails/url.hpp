#ifndef  CRAILS_URL_HPP
# define CRAILS_URL_HPP

# include <string>
# include <sstream>

class Data;

namespace Crails
{
  void cgi2params(const Data& params, const std::string& encoded_str);

  struct Url
  {
    static std::string encode(const std::string&);
    static std::string decode(const std::string&);
    static Url         from_string(const std::string&);
    std::string        to_string() const;

    bool           ssl = false;
    std::string    host;
    unsigned short port = 80;
    std::string    target;
  };

  struct UrlBuilder
  {
    template<typename T, typename... ARGS>
    static void fragments(std::stringstream& stream, T fragment, ARGS... right)
    {
      stream << Url::encode(fragment) << '/';
      fragments(stream, right...);
    }

    template<typename T>
    static void fragments(std::stringstream& stream, T fragment)
    {
      stream << Url::encode(fragment);
    }
  };

  template<typename... ARGS>
  std::string uri(ARGS... args)
  {
    std::stringstream stream;

    stream << '/';
    UrlBuilder::fragments(stream, args...);
    return stream.str();
  }
}

#endif
