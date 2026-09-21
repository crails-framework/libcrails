#include "url.hpp"
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <regex>
#include <limits>

using namespace std;
using namespace Crails;

Url Url::from_string(const std::string_view url)
{
  static const std::regex url_matcher("^(https?)://([^/:]+)(:([0-9]{1,5}))?/?(.*)$");
  match_results<string_view::const_iterator> matches;

  if (regex_match(url.begin(), url.end(), matches, url_matcher))
  {
    bool         ssl  = matches[1].str() == "https";
    unsigned int port = matches[4].str().length() > 0
      ? boost::lexical_cast<unsigned int>(matches[4].str())
      : static_cast<unsigned int>(ssl ? 443 : 80);

    if (port <= std::numeric_limits<unsigned short>::max())
    {
      return Url{
        /* ssl    */ ssl,
        /* host   */ matches[2].str(),
        /* port   */ static_cast<unsigned short>(port),
        /* target */ matches[5].str()
      };
    }
  }
  return Url{};
}

std::string Url::to_string() const
{
  ostringstream stream;

  stream << "http";
  if (ssl) stream << 's';
  stream << "://" << host << ':' << port << '/' << target;
  return stream.str();
}
