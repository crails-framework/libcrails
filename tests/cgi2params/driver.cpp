#include <crails/url.hpp>
#include <crails/datatree.hpp>

#undef NODEBUG
#include <cassert>

int main()
{
  using namespace Crails;
  using namespace std;

  // Single value
  {
    DataTree result;

    cgi2params(result.as_data(), "one=value");
    assert(result["one"].as<string>() == "value");
  }

  // Multiple values
  {
    DataTree result;

    cgi2params(result.as_data(), "one=value&two=values");
    assert(result["one"].as<string>() == "value");
    assert(result["two"].as<string>() == "values");
  }

  // Decodes characters
  {
    DataTree result;

    cgi2params(result.as_data(), "first=encoded%20value");
    assert(result["first"].as<string>() == "encoded value");
  }

  // Handle hashes
  {
    DataTree result;

    cgi2params(result.as_data(), "first%5B1%5D=coucou&first%5B2%5D=goodbye");
    assert(result["first"]["1"].as<string>() == "coucou");
    assert(result["first"]["2"].as<string>() == "goodbye");
  }

  // Handles arrays
  {
    DataTree result;
    vector<string> values;

    cgi2params(result.as_data(), "first%5B1%5D%5B%5D=coucou&first%5B1%5D%5B%5D=goodbye");
    values = result["first"]["1"].to_vector<string>();
    assert(values.size() == 2);
    assert(values[0] == "coucou");
    assert(values[1] == "goodbye");
  }

  return 0;
}
