///@file   world-imp.hpp
///@author Chris Newman
///@brief  Implementation of the World type.
#pragma once

#include <type_traits>
#include <memory>

#include "component-view.hpp"
#include "world.hpp"
#include "system.hpp"

namespace __detail
{
  template<typename ID_T>
  struct SystemPackage
  {
    struct Base
    {
      virtual void run(TWorld<ID_T>* w) = 0;
      virtual ~Base() = default;
    };
    template<typename SYS>
    struct Derived : Base
    {
      std::unique_ptr<SYS> sysPtr_ = nullptr;

      Derived(SYS* sysPtr)
        : sysPtr_(sysPtr) {}

      template<size_t... IS>
      void run_each_caller(TWorld<ID_T>* w, std::index_sequence<IS...>)
      {
        (w);
        if constexpr (SYS::compsNum > 0)
          (*sysPtr_)( w->template view_get<typename SYS::template CompType<IS>...>() );
        else
          (*sysPtr_)();
      }

      void run(TWorld<ID_T>* w) override
      {
        run_each_caller(w, std::make_index_sequence<SYS::compsNum>());
      }
    };

    std::unique_ptr<Base> bPtr_ = nullptr;
  };



  template <typename COMP, typename ID_T>
  TCompArray<COMP, ID_T>::TCompArray()
  {}

  // TCompArray ////////////////////////////////////////////////////////////////
  template<typename COMP, typename ID_T>
  template<typename ...ARGS>
  bool TCompArray<COMP, ID_T>::insert(ID_T id, ARGS&&... args)
  {
    // Check if the id is already added to the array
    if (map_.size() > id && map_[id] != std::numeric_limits<ID_T>::max())
      return false;
    
    // Otherwise, insert it
    try
    {
      arr_.push_back(COMP{ std::forward<ARGS>(args)... });
    }
    catch (std::exception ex)
    {
      printf(ex.what());
    }

    if (map_.size() <= id)
      map_.resize(static_cast<size_t>(id + 1), std::numeric_limits<ID_T>::max());
    map_[id] = static_cast<ID_T>(arr_.size() - 1);
    revMap_.push_back(id);
    return true;
  }

  template<typename COMP, typename ID_T>
  void TCompArray<COMP, ID_T>::remove(ID_T id)
  {
    if (map_.size() <= id || map_[id] == std::numeric_limits<ID_T>::max())
      return;
    size_t index = map_[id];

    arr_[index] = arr_.back();
    revMap_[index] = revMap_.back();



    arr_.pop_back();
    revMap_.pop_back();
    map_[id] = std::numeric_limits<ID_T>::max();

    if(index >= revMap_.size())
    {
      return;
    }

    if(revMap_.size() > 0 && revMap_[index] != std::numeric_limits<ID_T>::max())
    {
      map_[revMap_[index]] = static_cast<ID_T>(index);
    }
  }

  template<typename COMP, typename ID_T>
  COMP* TCompArray<COMP, ID_T>::get(ID_T id)
  {
    if (map_.size() <= id || map_[id] == std::numeric_limits<ID_T>::max() || arr_.size() <= map_[id])
      return nullptr;
    return &(arr_[map_[id]]);
  }

  template<typename COMP, typename ID_T>
  COMP* TCompArray<COMP, ID_T>::get_by_index(size_t i)
  {
    if (i >= arr_.size())
      return nullptr;
    return &(arr_[i]);
  }

  template<typename COMP, typename ID_T>
  ID_T TCompArray<COMP, ID_T>::get_id(size_t index)
  {
    return revMap_[index];
  }

  template<typename COMP, typename ID_T>
  ID_T TCompArray<COMP, ID_T>::get_id(COMP& comp)
  {
    return revMap_[&comp - arr_.data()];
  }

  template<typename COMP, typename ID_T>
  std::vector<COMP>& TCompArray<COMP, ID_T>::array() 
  {
    return arr_;
  }

  template<typename COMP, typename ID_T>
  bool TCompArray<COMP, ID_T>::contains(ID_T id)
  {
    return get(id);
  }

  template<typename COMP, typename ID_T>
  size_t TCompArray<COMP, ID_T>::index_of(ID_T ent)
  {
    if (map_.size() <= ent || map_[ent] == std::numeric_limits<ID_T>::max())
      return std::numeric_limits<size_t>::max();
    return map_[ent];
  }
  
  template<typename COMP, typename ID_T>
  size_t TCompArray<COMP, ID_T>::size() const
  {
    return arr_.size();
  }



  // TCompRegistry /////////////////////////////////////////////////////////////
  template<typename ID_T>
  template<typename COMP>
  TCompArray<COMP, ID_T>& TCompRegistry<ID_T>::get_array()
  {
    size_t const key = get_type_id<COMP, ComponentFamily>();

    if (reg_.size() <= key)
    {
      //reg_.emplace_back(new TCompArray<COMP, ID_T>{});
      reg_.resize(key + 1);
    }

    if (reg_[key] == nullptr)
    {
      reg_[key].reset(new TCompArray<COMP, ID_T>{});
    }

    std::unique_ptr<ICompArrayBase<ID_T>>& arr = reg_[key];
    return *dynamic_cast<TCompArray<COMP, ID_T>*>(arr.get());
  }

  template<typename ID_T>
  void TCompRegistry<ID_T>::remove_all_of(ID_T id)
  {
    for (auto& arr : reg_)
    {
      if(!arr.get())
      {
        continue;
      }
      if (arr->contains(id))
      {
        arr->remove(id);
      }
    }
  }

  // TWorld ////////////////////////////////////////////////////////////////////
  template<typename ID_T>
  void TWorld<ID_T>::entity_destroy(Entity e)
  {
    compReg_.remove_all_of(e.id_);
  }

  template<typename ID_T>
  void TWorld<ID_T>::entity_destroy_delayed(Entity e)
  {
    removeList_.push_back(e);
  }

  template<typename ID_T>
  void TWorld<ID_T>::process_remove()
  {
    for (Entity e : removeList_)
    {
      entity_destroy(e);
    }
    removeList_.clear();
  }

  template<typename ID_T>
  template<typename COMP, typename ...ARGS>
  void TWorld<ID_T>::comp_add(Entity e, ARGS&&... args)
  {
    TCompArray<COMP, ID_T>& compArr = compReg_.template get_array<COMP>();
    compArr.insert(e.id_, std::forward<ARGS>(args)...);
  }

  template<typename ID_T>
  template<typename COMP>
  void TWorld<ID_T>::comp_remove(Entity e)
  {
    TCompArray<COMP, ID_T>& compArr = compReg_.template get_array<COMP>();
    compArr.remove(e.id_);
  }

  template<typename ID_T>
  template<typename COMP>
  COMP* TWorld<ID_T>::comp_get(Entity e)
  {
    TCompArray<COMP, ID_T>& compArr = compReg_.template get_array<COMP>();
    return compArr.get(e.id_);
  }

  template<typename ID_T>
  template<typename COMP>
  TArrayView<COMP> TWorld<ID_T>::comp_get()
  {
    TCompArray<COMP, ID_T>& compArr = compReg_.template get_array<COMP>();
    return TArrayView<COMP>{compArr.array().data(), compArr.array().size()};
  }

  template<typename ID_T>
  template<typename COMP>
  typename TWorld<ID_T>::Entity TWorld<ID_T>::comp_get_entity(size_t index)
  {
    TCompArray<COMP, ID_T>& compArr = compReg_.template get_array<COMP>();
    return compArr.get_id(index);
  }

  template<typename ID_T>
  template<typename COMP>
  typename TWorld<ID_T>::Entity TWorld<ID_T>::comp_get_entity(COMP& comp)
  {
    using T = std::remove_const_t<COMP>;
    TCompArray<T, ID_T>& compArr = compReg_.template get_array<T>();
    return compArr.get_id(comp);
  }

  template<typename ID_T>
  template<typename SYS, typename...CON_ARGS>
  void TWorld<ID_T>::sys_add(EventTypes evT, CON_ARGS&&...args)
  {
    static_assert(std::is_base_of_v<SystemBaseType, SYS>);
    SystemPackage<ID_T> sp;
    sp.bPtr_ = std::make_unique<SystemPackage<ID_T>::template Derived<SYS>>(
      new SYS(args...)
    );

    switch (evT)
    {
    default:
    case EventTypes::none:
      break;
    case EventTypes::tick:
      sysTick_.push_back(std::move(sp));
      break;
    case EventTypes::tickBegin:
      sysTickBegin_.push_back(std::move(sp));
      break;
    case EventTypes::tickEnd:
      sysTickEnd_.push_back(std::move(sp));
      break;
    }
  }

  template<typename ID_T>
  template<typename T, typename FUNC>
  void TWorld<ID_T>::each_single(FUNC func)
  {
    /*for (T& t : comp_get<T>())
    {
      func(t);
    }*/
    TCompArray<T, ID_T>& compArr = compReg_.template get_array<T>();
    auto& vec = compArr.array();
    for(size_t i = 0; i < vec.size(); ++i)
    {
      func(vec[i]);
    }
  }

  template<typename ID_T>
  template<typename FUNC, typename T, size_t... IS>
  void TWorld<ID_T>::each_func_exec(FUNC func, T args, std::index_sequence<IS...>)
  {
    func(*std::get<IS>(args)...);
  }

  template<typename ID_T>
  template<typename COMP, typename FROM_COMP>
  COMP* TWorld<ID_T>::each_get_ptr(size_t i, TCompArray<FROM_COMP, ID_T>& fromCompArr, bool& noNull)
  {
    if (!noNull)
      return nullptr;
    auto& compArr = compReg_.template get_array<COMP>();
    auto ptr = compArr.get(fromCompArr.get_id(i));
    if (!ptr)
    {
      noNull = false;
      return nullptr;
    }
    return ptr;
  }

  template<typename ID_T>
  template<typename ...COMP, typename FUNC>
  void TWorld<ID_T>::each(FUNC func)
  {
    // Needs to supply at least one component
    static_assert(sizeof...(COMP) > 0, "One or more component names required");
    static_assert(std::is_invocable_v<FUNC, COMP&...>, "func must be callable type that takes given components as references.");

    // Call a simpler version of the function if there's only one component.
    if constexpr (sizeof...(COMP) == 1)
    {
      each_single<COMP...>(func);
      return;
    }
    else
    {
      auto& compArr = compReg_.template get_array<typename std::tuple_element<0, std::tuple<COMP...>>::type>();
      auto& firstVec = compArr.array();
      for (size_t i = 0; i < firstVec.size(); i++)
      {
        bool noNull = true;
        std::tuple<COMP*...> ptrs = std::make_tuple(each_get_ptr<COMP>(i, compArr, noNull)...);

        if (noNull)
        {
          // Execute the function...?
          each_func_exec(func, ptrs, std::make_index_sequence<sizeof...(COMP)>{});
        }
      }
    }
  }

  template<typename ID_T>
  void TWorld<ID_T>::tick()
  {
    for (auto& s : sysTickBegin_)
    {
      s.bPtr_->run(this);
    }
    for(auto& s : sysTick_)
    {
      s.bPtr_->run(this);
    }
    for (auto& s : sysTickEnd_)
    {
      s.bPtr_->run(this);
    }
  }

  template <typename ID_T>
  template <typename ...COMP>
  TComponentView<ID_T, COMP...> TWorld<ID_T>::view_get()
  {
    TComponentView<ID_T, COMP...> ret{ *this };

    if constexpr (sizeof...(COMP) == 0)
      return ret;

    // Very roundabout way of getting the number of components in one of the arrays.
    auto& compArr = compReg_.template get_array<typename std::tuple_element<0, std::tuple<std::remove_pointer_t<COMP>...>>::type>();
    auto& firstVec = compArr.array();
    for (size_t i = 0; i < firstVec.size(); i++)
    {
      bool noNull = true;
      ID_T ent = compArr.get_id(i);
      size_t indices[] = {
        ([&]
        {
          size_t i = 0;
          if (noNull)
          {
            i = compReg_.template get_array<std::remove_pointer_t<COMP>>().index_of(ent);
            if (i == std::numeric_limits<size_t>::max() && !std::is_pointer_v<COMP>)
              noNull = false;
          }
          return i;
        }())...
      };

      if (noNull)
      {
        // put the indices in the comp view
        for (size_t j = 0; j < sizeof...(COMP); ++j)
        {
          ret.ind_[j].push_back(indices[j]);
        }
      }
    }

    return ret;
  }


}
