#include <crails/url.hpp>
#include <crails/environment.hpp>
#include <iostream>

namespace Crails
{
  Environment environment = Production;
}

#undef NODEBUG
#include <cassert>

int main()
{
  using namespace Crails;
  using namespace std;

  {
    string result = uri("coucou", "petite", "perruche", 42);

    assert(result == "/coucou/petite/perruche/42");
  }

  {
    string result = uri("must", "encode[]", "all of this");

    assert(result == "/must/encode%5B%5D/all+of+this");
  }

  {
    string result = uri();

    assert(result == "/");
  }

  return 0;
}
