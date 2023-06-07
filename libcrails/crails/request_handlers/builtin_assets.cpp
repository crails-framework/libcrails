#include "builtin_assets.hpp"
#include "../mimetype.hpp"

using namespace Crails;
using namespace std;

bool BuiltinAssets::accepts(const HttpRequest& request) const
{
  const auto accepted = request.find(HttpHeader::accept_encoding);

  return accepted != request.end() &&
         accepted->value().find(compression_strategy.data()) != string::npos;
}

void BuiltinAssets::add(const string_view name, const char* data, size_t length)
{
  insert(pair<string_view, BuiltinAsset>(name, BuiltinAsset{data, length, get_mimetype(name.data())}));
}

BuiltinAssetsHandler::BuiltinAssetsHandler(const BuiltinAssets& library)
  : RequestHandler(string("builtin-assets:") + library.get_root_path().data()), library(library)
{
}

void BuiltinAssetsHandler::operator()(Context& context, function<void(bool)> callback) const
{
  const auto& request = context.connection->get_request();
  bool handled = false;

  if (request.method() == HttpVerb::get && library.accepts(request))
  {
    const string_view path(library.get_root_path());
    const string uri(request.target());

    if (uri.substr(0, path.length()) == path)
      handled = process(uri.substr(path.length()), context.response);
  }
  callback(handled);
}

bool BuiltinAssetsHandler::process(const string& path, BuildingResponse& response) const
{
  auto it = library.find(path);

  if (it != library.end())
  {
    response.set_header(HttpHeader::content_type, it->second.mimetype);
    response.set_header(HttpHeader::content_encoding, library.get_content_encoding().data());
    response.set_status_code(HttpStatus::ok);
    response.set_body(it->second.data, it->second.length);
    response.send();
    return true;
  }
  return false;
}
