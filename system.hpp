///@file   system.hpp
///@author Chris Newman
///@brief  Base class for systems used by the world object.
#pragma once

#include <tuple>
#include <cstdint>
#include <string>

#include "world.hpp"
#include "component-view.hpp"
#include "component-set.hpp"

class SystemBaseType {};

///@brief ECS system base type. Contains meta type data.
template<typename ID_T, typename ...COMPS>
class TSystem : public SystemBaseType
{
public:
  /// Read/write permissions for each component
  static constexpr bool permissions[] = {std::is_const_v<COMPS>..., false};
  /// A tuple of all component types operated on by this system.
  using CompTypesList = std::tuple<std::remove_const_t<COMPS>...>;
  /// Type corresponding to index I
  template<size_t I>
  using CompType = std::tuple_element_t<I, CompTypesList>;
  /// Number of component types operated on by this system.
  static constexpr size_t compsNum = std::tuple_size_v<CompTypesList>;
  //static_assert(compsNum > 0);

  virtual ~TSystem() = default;

  using ComponentView = TComponentView<ID_T, COMPS...>;

  virtual void operator()(ComponentView arg)
  {
    (arg);
    throw std::runtime_error{ "operator() not implemented!" };
  }

  virtual void operator()()
  {
    throw std::runtime_error{ "operator() not implemented!" };
  }
};

// Type that a system class should derive from.
template<typename...COMPS>
using System = TSystem<IdT, COMPS...>;



// Example of a system.
class SystemSample : public System<int32_t, int64_t*>
{
public:

  // ComponentView is a type alias contained by the base System type. It has all of
  // the template parameters already set.
  // If the system takes no components, operator() should take no arguments.
  void operator()(ComponentView cv) override
  {
    // Old behavior
    cv.each([](int32_t& i, int64_t* i2)
    {
      if (i2)
        printf((std::to_string(i) + " Yes!\n").c_str());
      else
        printf((std::to_string(i) + " No!\n").c_str());
    });

    // Access to enitity ID
    // you should normally use `auto` for this.
    for (ComponentSet<IdT, int32_t, int64_t*> s : cv)
    {
      printf(std::to_string(s.entity).c_str());
      printf("\n");
    }
    // You can also use operator[] on a ComponentView

    // World can be accessed through cv.source()
  }
};

namespace {
  inline void example_system_func() {
    World world;

    // Populate world with some entities.
    for (int32_t i = 0; i < 100; ++i) {
      World::Entity e = world.entity_new();
      world.comp_add<int32_t>(e, i);
      // Give every other entity an int64 component.
      if (i % 2)
        world.comp_add<int64_t>(e, i * 1000);
    }

    // Add the example system defined above to the world.
    world.sys_add<SystemSample>(World::EventTypes::tick);
    // Activate all attached systems.
    world.tick();
  }
}

