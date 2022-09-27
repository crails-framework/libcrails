#ifndef  CRAILS_SAFE_POINTER_HPP
# define CRAILS_SAFE_POINTER_HPP

# include <memory>
# ifndef __COMET_CLIENT__
#  include <crails/utils/backtrace.hpp>
#  define NULLPTR_EXCEPTION_BASE boost_ext::exception
# else
#  define NULLPTR_EXCEPTION_BASE std::exception
# endif
# ifndef safe_ptr_base
#  define safe_ptr_base std::shared_ptr
# endif

struct NullPointerException : public NULLPTR_EXCEPTION_BASE
{
public:
  const char* what() const throw()
  {
    return ("A null pointer has been dereferenced.");
  }
};

template<typename T>
class safe_ptr : public safe_ptr_base<T>
{
public:
  safe_ptr()
  {
  }

  safe_ptr(T* ptr) : safe_ptr_base<T>(ptr)
  {
  }

  template<typename CPY>
  safe_ptr(CPY& cpy) : safe_ptr_base<T>(cpy)
  {
  }

  safe_ptr& operator=(const safe_ptr_base<T>& cpy)
  {
    safe_ptr_base<T>::operator=(cpy);
    return *this;
  }

  T* operator->() const
  {
    if (this->get() == 0)
      throw NullPointerException();
    return (this->get());
  }

  T& operator*() const
  {
    if (this->get() == 0)
      throw NullPointerException();
    return (*this->get());
  }
};

# define SP(T) ::safe_ptr<T>

#endif
