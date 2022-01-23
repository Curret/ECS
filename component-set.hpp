///@file    component-set.hpp
///@author  Chris Newman
///@brief   Contains the definition for a simple template for holding a set of 
///         references to components belonging to an entity.
#pragma once

#include <type_traits>

namespace __detail
{
  template<typename T>
  using Entry = std::conditional_t<std::is_pointer_v<T>, T, std::add_lvalue_reference_t<T>>;
}

template<typename ID_T, typename...>
struct ComponentSet;

template<typename ID_T, typename A>
struct ComponentSet<ID_T, A>
{
  static inline const size_t size = 1;
  ID_T entity;
  __detail::Entry<A> a;
};

template<typename ID_T, typename A, typename B>
struct ComponentSet<ID_T, A, B>
{
  static inline const size_t size = 2;
  ID_T entity;
  __detail::Entry<A> a;
  __detail::Entry<B> b;
};

template<typename ID_T, typename A, typename B, typename C>
struct ComponentSet<ID_T, A, B, C>
{
  static inline const size_t size = 3;
  ID_T entity;
  __detail::Entry<A> a;
  __detail::Entry<B> b;
  __detail::Entry<C> c;
};

template<typename ID_T, typename A, typename B, typename C, typename D>
struct ComponentSet<ID_T, A, B, C, D>
{
  static inline const size_t size = 4;
  ID_T entity;
  __detail::Entry<A> a;
  __detail::Entry<B> b;
  __detail::Entry<C> c;
  __detail::Entry<D> d;
};

template<typename ID_T, typename A, typename B, typename C, typename D, typename E>
struct ComponentSet<ID_T, A, B, C, D, E>
{
  static inline const size_t size = 5;
  ID_T entity;
  __detail::Entry<A> a;
  __detail::Entry<B> b;
  __detail::Entry<C> c;
  __detail::Entry<D> d;
  __detail::Entry<E> e;
};

