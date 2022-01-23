///@file   typeid.hpp
///@author Chris Newman
///@brief  Simple thread-safe, sequential and numerical type ID.
#pragma once

#include <atomic>

namespace __detail
{
  // TTypeID ////////////////////////////////////////////////////////////////////
  template<typename FAM>
  class TTypeId
  {
    inline static std::atomic<uint32_t> lastUnused_;

    template<typename...>
    inline static const auto inner = lastUnused_++;

  public:
    template<typename ...T>
    inline static const uint32_t id = inner<std::decay_t<T>...>;
  };
} // namespace __detail


template<typename T>
inline uint32_t get_type_id()
{
  return __detail::TTypeId<struct family_defualt>::template id<T>;
}

/// FAM can be any arbitrary (even incomplete) type. Used to create 'families'
/// of type IDs.
template<typename T, typename FAM>
inline uint32_t get_type_id()
{
  return __detail::TTypeId<FAM>::template id<T>;
}

