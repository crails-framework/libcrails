#ifndef  NO_SESSION_STORE
# define NO_SESSION_STORE

# include "../session_store.hpp"

namespace Crails
{
  class NoSessionStore : public SessionStore
  {
    SESSION_STORE_IMPLEMENTATION(NoSessionStore)
  public:
    void       load(const HttpRequest&) override {}
    void       finalize(BuildingResponse&) override {}
    Data       to_data() override { return stub.as_data(); }
    const Data to_data() const override { return stub.as_data(); }
  private:
    DataTree   stub;
  };
}

#endif
