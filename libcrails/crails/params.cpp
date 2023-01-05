#include "params.hpp"
#include "session_store.hpp"
#include <filesystem>

using namespace std;
using namespace Crails;

Params::Params(void)
{
  session = SessionStore::Factory::create();
}

Params::~Params(void)
{
  for (const File& file : files)
    filesystem::remove(file.temporary_path);
}

const Params::File* Params::get_upload(const string& key) const
{
  Files::const_iterator it = find(files.begin(), files.end(), key);

  if (it != files.end())
    return (&(*it));
  return (0);
}
