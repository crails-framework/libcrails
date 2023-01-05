#include "session_store/no_session_store.hpp"

using namespace std;
using namespace Crails;

unique_ptr<SessionStore> SessionStore::Factory::make()
{
  return make_unique<NoSessionStore>();
}
