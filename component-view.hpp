///@file   component-view.hpp
///@author Chris Newman
///@brief  Class used to interact with components belonging to the
///        same entities.

#pragma once

#include "component-set.hpp"
#include <optional>


namespace __detail
{
  template<typename>
  class TWorld;
}

template<typename ID_T, typename ...COMPS>
class TComponentView
{
  friend class __detail::TWorld<ID_T>;
public:

  /// Iterator class for iterating through the sets contained by a
  /// ComponentView.
  class Iterator
  {
  public:

    ComponentSet<ID_T, COMPS...> operator*() { return source_[index_]; }

    Iterator& operator++() { index_++; return *this; }
    Iterator operator++(int) { Iterator i = *this; index_++; return i; }
    Iterator& operator--() { index_--; return *this; }
    Iterator operator--(int) { Iterator i = *this; index_--; return i; }

    bool operator==(Iterator const& o) { return index_ == o.index_; }
    bool operator!=(Iterator const& o) { return index_ != o.index_; }

  private:
    size_t index_;
    TComponentView<ID_T, COMPS...>& source_;

    Iterator(size_t i, TComponentView<ID_T, COMPS...>& source)
      : index_{ i }, source_{ source } {}
    Iterator(Iterator const& other)
      : index_{ other.index_ }, source_{ other.source_ } {}

    friend class TComponentView<ID_T, COMPS...>;
  };

  ///@brief Executes a given function for each set contained by the
  ///       ComponentView.
  template<typename F>
  void each(F func);

  ///@brief Returns the number of sets contained by the ComponentView
  size_t size() const;

  ///@brief Gets the set at the given index. The index does not correspond to
  ///       any value relating to the World object.
  ComponentSet<ID_T, COMPS...> operator[](size_t i);

  Iterator begin();
  Iterator end();

  ///@brief   Gets a component set belonging to the given entity.
  std::optional<ComponentSet<ID_T, COMPS...>> get_by_entity(ID_T ent);

  ///@brief Gets the World object that this view was created from.
  __detail::TWorld<ID_T>& source() const { return world_; }

private:
  static constexpr size_t numberComps = sizeof...(COMPS);

  __detail::TWorld<ID_T>& world_;
  std::vector<size_t> ind_[numberComps];

  explicit TComponentView(__detail::TWorld<ID_T>& world);
};

template<typename ID_T>
class TComponentView<ID_T>
{
  friend class __detail::TWorld<ID_T>;
public:
  ///@brief Returns the number of sets contained by the ComponentView
  static size_t size() { return 0; }

  ///@brief Gets the World object that this view was created from.
  __detail::TWorld<ID_T>& source() const { return world_; }

private:
  __detail::TWorld<ID_T>& world_;

  explicit TComponentView(__detail::TWorld<ID_T>& world)
    : world_{ world } {}
};


#include "component-view-imp.hpp"
