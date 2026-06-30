#include "http_response.hpp"
#include <algorithm>

using namespace std;
using namespace Crails;

void BuildingResponse::set_response(HttpStatus code, string&& body)
{
  set_body(std::move(body));
  set_status_code(code);
}

void BuildingResponse::set_response(HttpStatus code, const string_view body)
{
  set_body(body.data(), body.length());
  set_status_code(code);
}

void BuildingResponse::set_body(const char* str, size_t size)
{
  if (sent()) return ;
  auto&   response = get_raw_response();

  response.body().assign(str, size);
  response.prepare_payload();
}

void BuildingResponse::set_body(string&& value)
{
  if (sent()) return ;
  auto&   response = get_raw_response();

  response.body() = std::move(value);
  response.prepare_payload();
}

void BuildingResponse::send()
{
  if (!already_sent.test_and_set())
    connection.write();
}
