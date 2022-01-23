///@file   component-view-imp.hpp
///@author Chris Newman
///@brief  Implementation for ComponentView object.
#pragma once
#include <optional>

namespace __detail
{
  // Hacky solution so that the compiler will resolve arguments in the
  // expected order.
  struct OrderedCall
  {
    template <typename F, typename ...Args>
    OrderedCall(F&& f, Args&&... args)
    {
      std::forward<F>(f)(std::forward<Args>(args)...);
    }
  };

  template<bool>
  struct MaybeDeref;

  template<>
  struct MaybeDeref<true>
  {
    template<typename T>
    static T* deref(T* ptr)
    {
      return ptr;
    }
  };

  template<>
  struct MaybeDeref<false>
  {
    template<typename T>
    static T& deref(T* ptr)
    {
      return *ptr;
    }
  };
}

template<typename ID_T, typename ...COMPS>
TComponentView<ID_T, COMPS...>::TComponentView(__detail::TWorld<ID_T>& world)
  : world_{ world } {}

template <typename ID_T, typename ...COMPS>
template <typename F>
void TComponentView<ID_T, COMPS...>::each(F func)
{
  for (size_t i = 0; i < ind_[0].size(); ++i)
  {
    size_t j = 0;
    __detail::OrderedCall{
      func,
      (__detail::MaybeDeref<std::is_pointer_v<COMPS>>::deref
        (world_.
          compReg_.
          template get_array<std::remove_const_t<std::remove_pointer_t<COMPS>>>().
          get_by_index
          (
            ind_[j++][i]
          )
        )
      )...
    };
  }
}

template <typename ID_T, typename ...COMPS>
size_t TComponentView<ID_T, COMPS...>::size() const
{
  return ind_[0].size();
}

template <typename ID_T, typename ...COMPS>
ComponentSet<ID_T, COMPS...> TComponentView<ID_T, COMPS...>::operator[](size_t i)
{
  size_t j = 0;
  return
    ComponentSet<ID_T, COMPS...>{
      world_.template comp_get_entity<std::tuple_element_t<0, std::tuple<std::remove_const_t<std::remove_pointer_t<COMPS>>...>>>(ind_[0][i]),
      (__detail::MaybeDeref<std::is_pointer_v<COMPS>>::deref
        (
          world_.
          compReg_.
          template get_array<std::remove_const_t<std::remove_pointer_t<COMPS>>>().
          get_by_index
          (
            ind_[j++][i]
          )
        )
      )...
    };
}

template <typename ID_T, typename ...COMPS>
typename TComponentView<ID_T, COMPS...>::Iterator TComponentView<ID_T, COMPS...>::begin()
{
  return Iterator{ 0, *this };
}

template <typename ID_T, typename ...COMPS>
typename TComponentView<ID_T, COMPS...>::Iterator TComponentView<ID_T, COMPS...>::end()
{
  return Iterator{ size(), *this };
}

template <typename ID_T, typename ...COMPS>
std::optional<ComponentSet<ID_T, COMPS...>> TComponentView<ID_T, COMPS...>::get_by_entity(ID_T ent)
{
  try
  {
    return ComponentSet<ID_T, COMPS...>{ ent, __detail::MaybeDeref<std::is_pointer_v<COMPS>>::deref(world_.template comp_get<std::remove_const_t<std::remove_pointer_t<COMPS>>>(ent))... };
  }
  catch (...)
  {
    return std::nullopt;
  }
}
