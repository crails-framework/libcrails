#ifndef  CRAILS_SAFE_POINTER_HPP
# define CRAILS_SAFE_POINTER_HPP

# include <memory>
# include <crails/utils/backtrace.hpp>
# ifndef safe_ptr_base
#  define safe_ptr_base std::shared_ptr
# endif

struct NullPointerException : public boost_ext::exception
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
