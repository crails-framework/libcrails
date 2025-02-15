#include <crails/mimetype.hpp>
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

  assert(get_mimetype("file.html") == "text/html");
  assert(get_mimetype("PICTURE.JPEG") == "image/jpg");
  assert(get_mimetype("Music.Opus") == "audio/ogg");
  assert(get_mimetype("index.html.gz") == "text/html");
  assert(get_mimetype("document.pDF") == "application/pdf");
  return 0;
}
