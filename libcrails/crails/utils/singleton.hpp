#ifndef  SINGLETON_HPP
# define SINGLETON_HPP

# include <crails/utils/backtrace.hpp>

# define SINGLETON(type) \
public:\
  typedef Singleton<type> singleton; \
private:\
  friend class Singleton<type>;

template<typename TYPE, typename... Args>
class Singleton
{
public:
  static void initialize(Args... args)
  {
    if (!(ptr))
      ptr = new TYPE(args...);
    else
      throw boost_ext::runtime_error("Was already initialized");
  }

  static void finalize(void)
  {
    if (ptr)
    {
      delete ptr;
      ptr = 0;
    }
  }

  static TYPE*  get(void) { return (ptr); }

private:
  static TYPE* ptr;
};

template<typename TYPE, typename... Args>
struct SingletonInstantiator
{
  SingletonInstantiator(Args... args)
  {
    Singleton<TYPE, Args...>::initialize(args...);
  }

  ~SingletonInstantiator()
  {
    Singleton<TYPE, Args...>::finalize();
  }

  TYPE* operator->() const { return Singleton<TYPE, Args...>::get(); }
};

template<typename TYPE, typename... Args> TYPE* Singleton<TYPE, Args...>::ptr = 0;

#endif
