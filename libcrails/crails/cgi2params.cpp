#include <vector>
#include <list>
#include <string>
#include <string_view>
#include <algorithm>
#include <optional>
#include "url.hpp"
#include <crails/datatree.hpp>
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

static const string_view opening_bracket = "%5B";
static const string_view closing_bracket = "%5D";
static constexpr size_t max_key_depth = 32;

static void assign_value(Data root, const vector<string>& key_stack, const string_view raw_value)
{
  list<DataTree>                owned_subtrees;
  vector<pair<Data, DataTree*>> pending_pushes;
  Data                          current = root;

  for (const string& encoded_key : key_stack)
  {
    const string key = Url::decode(encoded_key);

    if (key.empty())
    {
      owned_subtrees.emplace_back();
      DataTree& subtree = owned_subtrees.back();

      pending_pushes.emplace_back(current, &subtree);
      current = subtree.as_data();
    }
    else
      current = current[key];
  }
  current = Url::decode(raw_value);
  for (auto it = pending_pushes.rbegin() ; it != pending_pushes.rend() ; ++it)
    it->first.push_back(it->second->as_data());
}

static inline void parse_base_key(string_view str, size_t& pos, vector<string>& key_stack)
{
  size_t stop = min({
    str.find('=', pos),
    str.find('&', pos),
    str.find(opening_bracket, pos)
  });

  if (stop == string_view::npos)
    stop = str.length();
  key_stack.push_back(string(str.substr(pos, stop - pos)));
  pos = stop;
}

static inline bool parse_subscripts(string_view str, size_t& pos, vector<string>& key_stack)
{
  size_t end_bracket;

  while (pos + opening_bracket.length() <= str.length()
     && str.compare(pos, opening_bracket.length(), opening_bracket) == 0)
  {
    pos += opening_bracket.length();
    end_bracket = str.find(closing_bracket, pos);
    if (end_bracket == string_view::npos)
    {
      logger << Logger::Info << "# cgi2params: unterminated '[' in parameter, aborting query parsing" << Logger::endl;
      return false;
    }
    if (key_stack.size() >= max_key_depth)
    {
      logger << Logger::Info << "# cgi2params: nesting exceeds " << max_key_depth << " levels, aborting query parsing" << Logger::endl;
      return false;
    }
    key_stack.push_back(string(str.substr(pos, end_bracket - pos)));
    pos = end_bracket + closing_bracket.length();
  }
  return true;
}

static inline void parse_value_and_assign(Data params, string_view str, size_t& pos, vector<string>& key_stack)
{
  size_t value_end;

  pos++; // Skip '='
  value_end = str.find('&', pos);
  if (value_end == string_view::npos)
    value_end = str.length();
  assign_value(params, key_stack, str.substr(pos, value_end - pos));
  pos = value_end;
}

namespace Crails
{
  void cgi2params(Data params, const string_view str)
  {
    size_t pos = 0;

    while (pos < str.length())
    {
      vector<string> key_stack;

      parse_base_key(str, pos, key_stack);
      if (!parse_subscripts(str, pos, key_stack))
        return;
      if (pos < str.length() && str[pos] == '=')
        parse_value_and_assign(params, str, pos, key_stack);
      if (pos < str.length() && str[pos] == '&')
        pos++;
    }
  }
}
