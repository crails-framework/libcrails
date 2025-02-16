#include <crails/any_cast.hpp>
#include <crails/environment.hpp>
#include <iostream>

namespace Crails
{
  Environment environment = Test;
}

#undef NODEBUG
#include <cassert>

int main()
{
  using namespace Crails;
  using namespace std;

  // indiscriminately casts std::string, std::string_view and const char* to std::string
  {
    const string_view a("coucou");
    const string b("coucou");

    any constchar_value = "coucou";
    any stringview_value = string_view("coucou");
    any string_value = string("coucou");
    any const_stringview_value = a;
    any const_string_value = b;

    assert(Crails::any_cast(constchar_value) == "coucou");
    assert(Crails::any_cast(stringview_value) == "coucou");
    assert(Crails::any_cast(string_value) == "coucou");
    assert(Crails::any_cast(const_stringview_value) == "coucou");
    assert(Crails::any_cast(const_string_value) == "coucou");
  }

  // defaults_to
  {
    map<string,any> data{
      {"string", std::string("valueA")},
      {"string_view", std::string_view("valueB")},
      {"const char*", "valueC"},
      {"number", 42},
      {"float", 21.12},
      {"bool", true}
    };

    // Any string to std::string
    assert(Crails::defaults_to<string>(data, "string", "kek") == "valueA");
    assert(Crails::defaults_to<string>(data, "string_view", "kek") == "valueB");
    assert(Crails::defaults_to<string>(data, "const char*", "kek") == "valueC");

    // Any string to std::string_view
    assert(Crails::defaults_to<string_view>(data, "string", "kek") == "valueA");
    assert(Crails::defaults_to<string_view>(data, "string_view", "kek") == "valueB");
    assert(Crails::defaults_to<string_view>(data, "const char*", "kek") == "valueC");

    // Scalars
    assert(Crails::defaults_to<int>(data, "number", 12) == 42);
    assert(Crails::defaults_to<double>(data, "float", 12) == 21.12);
    assert(Crails::defaults_to<bool>(data, "bool", false) == true);

    // Fallback to default value if cast type is wrong
    assert(Crails::defaults_to<double>(data, "number", 42.24) == 42.24);

    // Fallback to default value if no matching variable found
    assert(Crails::defaults_to<string>(data, "jagon", "sucer") == "sucer");
  }
  return 0;
}

