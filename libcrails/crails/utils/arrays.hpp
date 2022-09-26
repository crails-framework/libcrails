#ifndef  LIBCRAILS_ARRAYS_HPP
# define LIBCRAILS_ARRAYS_HPP

# include <set>
# include <vector>

namespace Crails
{
  template<typename LIST>
  LIST& to_unique_list(LIST& list)
  {
    std::set<typename LIST::value_type> unique_items;

    std::copy(
      std::make_move_iterator(list.begin()),
      std::make_move_iterator(list.end()),
      std::inserter(unique_items, unique_items.end()));
    list.clear();
    std::copy(
      std::make_move_iterator(unique_items.begin()),
      std::make_move_iterator(unique_items.end()),
      std::back_inserter(list));
    return list;
  }

  template<typename LIST>
  LIST unique_list(const LIST& source)
  {
    LIST list(source.begin(), source.end());

    to_unique_list<LIST>(list);
    return list;
  }

  template<typename LIST>                                                                                             
  LIST exclude_nullptr(const LIST& list)
  {                                                                                                                   
    LIST results;
  
    std::copy_if(
      list.begin(),                                                                                                   
      list.end(),
      std::inserter(results, results.end()),
      [](const typename LIST::value_type entry) { return entry != nullptr; });                                        
    return results;                                                                                                   
  }

  template<typename LIST, typename RETURN_TYPE, typename... ARGS>
  std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (LIST::value_type::*method)(ARGS...) const noexcept, ARGS... args)
  {
    std::vector<RETURN_TYPE> results;

    results.reserve(source.size());
    for (const auto& item : source)
      results.push_back((item.*method)(args...));
    return results;
  }
  template<typename LIST, typename RETURN_TYPE, typename... ARGS>
  std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (LIST::value_type::*method)(ARGS...) const, ARGS... args)
  {
    std::vector<RETURN_TYPE> results;

    results.reserve(source.size());
    for (const auto& item : source)
      results.push_back((item.*method)(args...));
    return results;
  }

  template<typename LIST, typename RETURN_TYPE, typename... ARGS>
  std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (std::pointer_traits<typename LIST::value_type>::element_type::*method)(ARGS...) const noexcept, ARGS... args)
  {
    std::vector<RETURN_TYPE> results;

    results.reserve(source.size());
    for (const auto& item : exclude_nullptr(source))
      results.push_back(((*item).*method)(args...));
    return results;
  }
  template<typename LIST, typename RETURN_TYPE, typename... ARGS>
  std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (std::pointer_traits<typename LIST::value_type>::element_type::*method)(ARGS...) const, ARGS... args)
  {
    std::vector<RETURN_TYPE> results;

    results.reserve(source.size());
    for (const auto& item : exclude_nullptr(source))
      results.push_back(((*item).*method)(args...));
    return results;
  }
}

#endif
