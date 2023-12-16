#include "../request_handlers/file.hpp"
#include "../server.hpp"
#include "../mimetype.hpp"
#include "../http.hpp"
#include <crails/utils/string.hpp>
#include <crails/logger.hpp>
#include <filesystem>
#include <chrono>
#include <boost/lexical_cast.hpp>
#include <time.h>
#include <string_view>

using namespace std;
using namespace Crails;

const vector<pair<string, string> > compression_strategies = {{"br", "br"}, {"gzip", "gz"}};

static string filepath_from_uri(string uri)
{
  std::error_code ec;
  size_t separator = uri.find('?');
  filesystem::path result;

  if (separator != std::string::npos)
    uri.erase(separator);

  for (const string& public_path : Server::get_public_paths())
  {
    auto canonical_path = filesystem::canonical(public_path + uri, ec);

    if (!ec && canonical_path >= public_path)
      return canonical_path.string();
    else
      logger << Logger::Info << "# Attempting to read unauthorized path '" << canonical_path.string() << Logger::endl;
  }
  return "";
}

static bool accepts_encoding(const HttpRequest& request, const std::string& name)
{
  const auto accepted = request.find(HttpHeader::accept_encoding);

  return accepted != request.end() && accepted->value().find(name) != string::npos;
}

static void serve_compressed_file_if_possible(string& fullpath, BuildingResponse& response, const HttpRequest& request)
{
  for (const auto& strategy : compression_strategies)
  {
    if (accepts_encoding(request, strategy.first) && filesystem::exists(fullpath + '.' + strategy.second))
    {
      response.set_header(HttpHeader::content_encoding, strategy.first);
      fullpath += '.' + strategy.second;
      break ;
    }
  }
}

static std::string make_header_string(boost::beast::string_view header)
{
  string header_string(header);
  auto pos = header_string.rfind("\r\n");

  while (pos != std::string::npos)
  {
    header_string = header_string.substr(0, pos);
    pos = header_string.rfind("\r\n");
  }
  return header_string;
}

static std::pair<unsigned int, unsigned int> range_from_header(boost::beast::string_view header)
{
  auto parts = split(make_header_string(header), '=');
  pair<unsigned int, unsigned int> range{0,0};

  if (parts.size() == 2)
  {
    string range_part = *parts.rbegin();

    parts = split(range_part, '-');
    if (parts.begin()->length() > 0 && range_part[0] != '-')
      range.first  = boost::lexical_cast<unsigned int>(*parts.begin());
    if (parts.rbegin()->length() > 0 && (*range_part.rbegin()) != '-')
      range.second = boost::lexical_cast<unsigned int>(*parts.rbegin());
  }
  return range;
}

static std::string get_content_range(pair<unsigned int, unsigned int> range, size_t length)
{
  std::stringstream result;

  result << "bytes ";
  if (range.first + range.second == 0)
    result << "0-" << (length - 1);
  else
  {
    if (range.first > 0) result << range.first;
    result << '-';
    if (range.second > 0)
      result << range.second;
    else
      result << range.first + (length - range.first) - 1;
  }
  result << '/' << length;
  return result.str();
}

static std::time_t http_date_to_timestamp(boost::beast::string_view str)
{
  static const char format[] = "%a, %d %b %Y %H:%M:%S %Z"; // rfc 1123
  struct tm tm;
  bzero(&tm, sizeof(tm));
  if (strptime(str.data(), format, &tm))
    return mktime(&tm);
  return 0;
}

template <typename TP>
static std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
              + system_clock::now());
    return system_clock::to_time_t(sctp);
}

static bool if_not_modified(boost::beast::string_view str_time, BuildingResponse& response, const string& fullpath)
{
  time_t condition_time = http_date_to_timestamp(str_time);
  time_t modified_time  = to_time_t(filesystem::last_write_time(fullpath));
//time_t modified_time  = chrono::system_clock::to_time_t(chrono::file_clock::to_sys(filesystem::last_write_time(fullpath)));

  if (modified_time <= condition_time)
  {
    response.set_status_code(boost::beast::http::status::not_modified);
    return true;
  }
  return false;
}

void FileRequestHandler::operator()(Context& context, function<void(bool)> callback) const
{
  bool result = context.connection->get_request().method() == HttpVerb::get
             && process(context);

  callback(result);
}

bool FileRequestHandler::process(Context& context) const
{
  const auto&  request = context.connection->get_request();
  const string uri(request.target());
  string       fullpath = filepath_from_uri(uri);
  auto         success_status = HttpStatus::ok;

  if (fullpath.length() == 0)
    context.response.set_status_code(HttpStatus::not_found);
  else
  {
    Range      range{0,0};
    const auto range_field  = request.find(HttpHeader::range);
    const auto update_field = request.find(HttpHeader::if_modified_since);

    if (filesystem::is_directory(fullpath))
      fullpath += "/index.html";
    serve_compressed_file_if_possible(fullpath, context.response, request);
    if (range_field != request.end())
    {
      range = range_from_header(range_field->value().data());
      success_status = HttpStatus::partial_content;
    }
    if (update_field != request.end() && if_not_modified(update_field->value(), context.response, fullpath))
    {
      context.response.set_status_code(HttpStatus::not_modified);
      return true;
    }
    else if (send_file(fullpath, context.response, success_status, range))
      return true;
  }
  return false;
}

bool FileRequestHandler::send_file(const std::string& fullpath, BuildingResponse& response, HttpStatus code, std::pair<unsigned int, unsigned int> range) const
{
  FileCache::Lock lock(file_cache);
  bool cached = file_cache.contains(fullpath);
  auto file   = cache_enabled ? file_cache.require(fullpath) : file_cache.create_instance(fullpath);

  if (file)
  {
    const string& str = *file;

    response.set_header(HttpHeader::content_type, get_mimetype(fullpath));
    if (code == HttpStatus::partial_content)
    {
      response.set_header(HttpHeader::accept_ranges, "bytes");
      response.set_header(HttpHeader::content_range, get_content_range(range, str.length()));
    }
    set_headers_for_file(response, fullpath);
    response.set_status_code(code);
    if (range.second == 0)
      range.second = str.size();
    response.set_body(str.c_str() + range.first, range.second - range.first);
    logger << Logger::Info << "# Delivering asset `" << fullpath << "` ";
    if (cache_enabled)
      logger << (cached ? "(was cached)" : "(was not cached)") << Logger::endl;
    else
    {
      file_cache.garbage_collect();
      logger << "(cache disabled)" << Logger::endl;
    }
    return true;
  }
  return false;
}
