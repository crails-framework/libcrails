#ifndef  CRAILS_HTTP_TYPES_HPP
# define CRAILS_HTTP_TYPES_HPP

# include <boost/beast/http.hpp>

namespace Crails
{
  typedef boost::beast::http::request<boost::beast::http::buffer_body>  HttpRequest;
  typedef boost::beast::http::response<boost::beast::http::string_body> HttpResponse;
  typedef boost::beast::http::status                                    HttpStatus;
  typedef boost::beast::http::field                                     HttpHeader;
  typedef boost::beast::http::verb                                      HttpVerb;
}

#endif
