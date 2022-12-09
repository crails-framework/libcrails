#ifndef  SINGLETON_HPP
# define SINGLETON_HPP

# include <crails/utils/backtrace.hpp>

# define SINGLETON(type) \
public:\
  typedef Singleton<type> singleton; \
private:\
  friend class Singleton<type>;

# define SINGLETON_IMPLEMENTATION(type, from_type) \
public:\
  typedef SingletonImplementation<type, from_type> singleton;\
private:\
  friend class SingletonImplementation<type, from_type>;

template<typename TYPE>
class Singleton
{
public:
  template<typename... Args>
  static void initialize(Args... args)
  {
    if (!(ptr))
      ptr = new TYPE(args...);
    else
      throw boost_ext::runtime_error("Was already initialized");
  }

  template<typename IMPLEMENTATION, typename... Args>
  static void implement(Args... args)
  {
    if (!(ptr))
      ptr = new IMPLEMENTATION(args...);
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

template<typename TYPE, typename FROM>
class SingletonImplementation
{
public:
  template<typename... Args>
  static void initialize(Args... args)
  {
    FROM::singleton::template implement<TYPE>(args...);
  }

  static void finalize(void)
  {
    FROM::singleton::finalize();
  }

  static TYPE* get(void) { return FROM::singleton::get(); }
};

template<typename TYPE>
struct SingletonInstantiator
{
  template<typename... Args>
  SingletonInstantiator(Args... args)
  {
    Singleton<TYPE>::initialize(args...);
  }

  SingletonInstantiator(const SingletonInstantiator&) = delete;

  ~SingletonInstantiator()
  {
    Singleton<TYPE>::finalize();
  }

  TYPE* operator->() const { return Singleton<TYPE>::get(); }
};

template<typename TYPE> TYPE* Singleton<TYPE>::ptr = 0;

#endif
