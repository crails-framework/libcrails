#include "request_parser.hpp"
#include "params.hpp"

using namespace std;
using namespace Crails;

bool RequestParser::content_type_matches(const HttpRequest& request, const regex regexp) const
{
  auto content_type_header = request.find(HttpHeader::content_type);

  if (content_type_header != request.end())
  {
    const auto& data = content_type_header->value();

    return regex_search(data.begin(), data.end(), regexp);
  }
  return false;
}

 
