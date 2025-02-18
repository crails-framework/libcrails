#include <regex>
#include <string>

using namespace std;

struct ExtensionMatch
{
  ExtensionMatch(const std::string& extension, const std::string& mime) :
    pattern(extension),
    regexp(extension + '$', std::regex_constants::ECMAScript | std::regex_constants::icase),
    mime(mime)
  {
  }

  std::string pattern;
  regex       regexp;
  std::string mime;
};
  
static const ExtensionMatch extensions[] = {
  ExtensionMatch("(htm|html)(.gz|.br)?", "text/html"),
  ExtensionMatch("js(.gz|.br)?",         "text/javascript"),
  ExtensionMatch("css(.gz|.br)?",        "text/css"),
  ExtensionMatch("png",                  "image/png"),
  ExtensionMatch("(jpg|jpeg)",           "image/jpg"),
  ExtensionMatch("webp",                 "image/webp"),
  ExtensionMatch("avif",                 "image/avif"),
  ExtensionMatch("gif",                  "image/gif"),
  ExtensionMatch("bmp",                  "image/bmp"),
  ExtensionMatch("svg",                  "image/svg+xml"),
  ExtensionMatch("txt",                  "text/txt"),
  ExtensionMatch("o(pus|gg|ga)",         "audio/ogg"),
  ExtensionMatch("mp3",                  "audio/mpeg"),
  ExtensionMatch("ogv",                  "video/ogg"),
  ExtensionMatch("mp4",                  "video/mp4"),
  ExtensionMatch("webm",                 "video/webm"),
  ExtensionMatch("pdf",                  "application/pdf")
};

namespace Crails
{
  string get_mimetype(const string& filename)
  {
    constexpr int count = sizeof(extensions) / sizeof(ExtensionMatch);

    for (unsigned short i = 0 ; i < count ; ++i)
    {
      if (regex_search(filename, extensions[i].regexp))
        return (extensions[i].mime);
    }
    return ("application/octet-stream");
  }
}
