#ifndef  LIBCRAILS_ARRAYS_HPP
# define LIBCRAILS_ARRAYS_HPP

# include <set>
# include <vector>
# include <algorithm>
# include <memory>
# include "type_traits.hpp"

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

  template<typename LIST, bool IS_POINTER, typename ...ARGS>
  class ArrayCollector
  {
    ArrayCollector() = delete;
  public:
    template<typename RETURN_TYPE, typename METHOD_SCOPE>
    static std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (METHOD_SCOPE::*method)(ARGS...) const, ARGS... args)
    {
      std::vector<RETURN_TYPE> results;
    
      results.reserve(source.size());
      for (const auto& item : source)
        results.push_back((item.*method)(args...));
      return results;
    }
  };

  template<typename LIST, typename ...ARGS>
  class ArrayCollector<LIST, true, ARGS...>
  {
    ArrayCollector() = delete;
  public:
    template<typename RETURN_TYPE, typename METHOD_SCOPE>
    static std::vector<RETURN_TYPE> collect(const LIST& source, RETURN_TYPE (METHOD_SCOPE::*method)(ARGS...) const, ARGS... args)
    {
      std::vector<RETURN_TYPE> results;
      LIST list_without_nullptrs = exclude_nullptr(source);
    
      results.reserve(list_without_nullptrs.size());
      for (const auto& item : list_without_nullptrs)
        results.push_back(((*item).*method)(args...));
      return results;
    }
  };

  template<typename LIST, typename METHOD, typename ...ARGS>
  auto collect(const LIST& source, METHOD method, ARGS... args)
  {
      return ArrayCollector<
        LIST,
        std::is_pointer<typename LIST::value_type>::value || is_specialization<typename LIST::value_type, std::shared_ptr>::value,
        ARGS...
      >::collect(source, method, args...);
  }
}

#endif
