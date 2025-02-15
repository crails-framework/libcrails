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

  template<typename T>
  struct UrlFragmentEncode { static inline std::string run(T fragment) { return std::to_string(fragment); } };
  template<>
  struct UrlFragmentEncode<std::string_view> { static inline std::string run(const std::string_view fragment) { return Url::encode(std::string(fragment)); } };
  template<>
  struct UrlFragmentEncode<std::string> { static inline std::string run(const std::string& fragment) { return Url::encode(fragment); } };
  template<>
  struct UrlFragmentEncode<const char*> { static inline std::string run(const char* fragment) { return Url::encode(fragment); } };

  struct UrlBuilder
  {
    template<typename T, typename... ARGS>
    static void fragments(std::stringstream& stream, T fragment, ARGS... right)
    {
      stream << UrlFragmentEncode<T>::run(fragment) << '/';
      fragments(stream, right...);
    }

    template<typename T>
    static void fragments(std::stringstream& stream, T fragment)
    {
      stream << UrlFragmentEncode<T>::run(fragment);
    }

    static void fragments(std::stringstream&) {}
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
