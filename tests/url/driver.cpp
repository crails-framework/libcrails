#include <crails/url.hpp>

#undef NODEBUG
#include <cassert>

int main()
{
  using namespace Crails;
  using namespace std;

  {
    string result = Url::decode("/must/decode%5B%5D/all%20of%20this");

    assert(result == "/must/decode[]/all of this");
  }

  {
    string result = Url::encode("/must/encode[]/all of this");

    assert(result == "%2Fmust%2Fencode%5B%5D%2Fall+of+this");
  }

  {
    Url url = Url::from_string("https://github.com/crails-framework/crails");

    assert(url.ssl == true);
    assert(url.host == "github.com");
    assert(url.port == 443);
    assert(url.target == "crails-framework/crails");
  }

  {
    Url url = Url::from_string("http://github.com:3030");

    assert(url.ssl == false);
    assert(url.host == "github.com");
    assert(url.port == 3030);
    assert(url.target == "");
  }

  {
    Url url;

    url.ssl = true;
    url.port = 8080;
    url.host = "localhost";
    url.target = "hello";
    assert(url.to_string() == "https://localhost:8080/hello");
  }

  return 0;
}
