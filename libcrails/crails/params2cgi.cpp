#include <crails/datatree.hpp>
#include <sstream>
#include "url.hpp"

using namespace std;
using namespace Crails;

static std::string get_form_path(Data data)
{
  const std::string& path = data.get_path();
  std::string        value;
  bool               first = true;

  value.reserve(path.length() * 2);
  for (int i = 0 ; i < path.length() ; ++i)
  {
    if (path[i] == '.')
    {
      if (!first)
        value += ']';
      value += '[';
      first = false;
    }
    else
      value += path[i];
  }
  if (!first)
    value += ']';
  return value;
}

namespace Crails
{
  void params2cgi(std::ostream& stream, Data params, bool first = true)
  {
    if (params.is_array())
    {
      for (Data entry : params)
      {
        if (!first) stream << '&';
        stream << Url::encode(get_form_path(params)) << "%5B%5D=" << Url::encode(entry.as<string>());
        first = false;
      }
    }
    else if (params.get_keys().size() > 0)
    {
      for (Data entry : params)
        params2cgi(stream, entry, first);
    }
    else
    {
      if (!first) stream << '&';
      stream << Url::encode(get_form_path(params)) << '=' << Url::encode(params.as<string>());
    }
  }

  std::string params2cgi(Data params)
  {
    std::ostringstream stream;

    params2cgi(stream, params);
    return stream.str();
  }
}
