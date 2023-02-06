#ifndef  CRAILS_TYPE_TRAITS_HPP
# define CRAILS_TYPE_TRAITS_HPP

# include <type_traits>

namespace Crails
{
  // Based on Databyte answer from:
  // https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector/74845006
  template<typename Test, template<typename...> class Ref>
  struct is_specialization : std::false_type {};

  template<template<typename...> class Ref, typename... Args>
  struct is_specialization<Ref<Args...>, Ref>: std::true_type {};
}

#endif
