#include <vector>
#include <regex>
#include "url.hpp"
#include "datatree.hpp"
#include <string_view>

using namespace std;
using namespace Crails;

static const string_view opening_bracket = "%5B";
static const string_view closing_bracket = "%5D";
static const regex regexp("[^=&]*", regex_constants::ECMAScript | regex_constants::optimize);

static void recursively_set_value(Data param, vector<string> key_stack, const string& value)
{
  if (key_stack.size() == 0)
    param = Url::decode(value);
  else
  {
    string key = Url::decode(key_stack.front());

    key_stack.erase(key_stack.begin());
    if (key.length() == 0)
    {
      DataTree sub_object;

      recursively_set_value(sub_object.as_data(), key_stack, value);
      param.push_back(sub_object.as_data());
    }
    else
    {
      Data next = param[key];

      recursively_set_value(next, key_stack, value);
    }
  }
}

namespace Crails
{
  void cgi2params(const Data& params, const string& encoded_str)
  {
    string str(encoded_str);
    string looping;
    vector<string> key_stacks;

    while (str.length())
    {
      if (str == looping)
        return ;
      auto matches = sregex_iterator(str.begin(), str.end(), regexp);

      looping = str;
      if (distance(matches, sregex_iterator()) > 0)
      {
        auto sub_key = str.find(opening_bracket);
        smatch match = *matches;
        auto   eo    = match.length(0);

        if (sub_key != string::npos && (int)sub_key < match.position(0) + eo)
          eo = sub_key - match.position(0);
        key_stacks.push_back(str.substr(match.position(0), eo));
        str.erase(match.position(0), eo);
        while (str.find(opening_bracket) == 0)
        {
          str.erase(0, opening_bracket.length());
          auto end_key = str.find(closing_bracket);
          if (end_key != string::npos)
          {
            key_stacks.push_back(str.substr(0, end_key));
            str.erase(0, end_key + closing_bracket.length());
          }
          else
            return ;
        }
        if (str[0] == '=')
        {
          str.erase(0, 1);
          matches = sregex_iterator(str.begin(), str.end(), regexp);
          if (distance(matches, sregex_iterator()) > 0)
          {
            match = *matches;
            recursively_set_value(params, key_stacks, str.substr(match.position(0), match.length(0)));
            key_stacks.clear();
            str.erase(match.position(0), match.length(0));
          }
        }
        if (str[0] == '&')
          str.erase(0, 1);
      }
      else
        return ;
    }
  }
}
