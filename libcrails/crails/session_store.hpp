#ifndef  SESSION_STORE_HPP
# define SESSION_STORE_HPP

# include "http.hpp"
# include <crails/datatree.hpp>
# include <crails/utils/singleton.hpp>

# define SESSION_STORE_IMPLEMENTATION(classname) \
  public: \
  class Factory : public SessionStore::Factory \
  { \
    SINGLETON_IMPLEMENTATION(classname, SessionStore::Factory) \
    std::unique_ptr<SessionStore> make() override { return std::make_unique<classname>(); } \
  }; \
  private:

namespace Crails
{
  class BuildingResponse;

  class SessionStore
  {
  public:
    virtual ~SessionStore() {}

    virtual void load(const HttpRequest&) = 0;
    virtual void finalize(BuildingResponse&) = 0;
    virtual Data to_data(void) = 0;
    virtual const Data to_data(void) const = 0;

    class Factory
    {
      SINGLETON(Factory)
    public:
      static std::unique_ptr<SessionStore> create()
      {
        return Factory::singleton::require().make();
      }
    protected:
      virtual std::unique_ptr<SessionStore> make();
    };
  };
}

#endif
