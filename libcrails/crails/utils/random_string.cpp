#include "random_string.hpp"
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace Crails;
using namespace std;

static const string default_random_charset = "abcdefghijklmnopqrstuvwxyz"
                                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                             "0123456789,.;:!?&%$#()[]{}+-";

namespace Crails
{
  string generate_random_string(unsigned short length)
  {
    return generate_random_string(default_random_charset, length);
  }

  string generate_random_string(const string& charset, unsigned short length)
  {
    string result;
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, charset.size() - 1);

    result.reserve(length);
    for (unsigned short i = 0 ; i < length ; ++i)
      result += charset[index_dist(rng)];
    return result;
  }
}
