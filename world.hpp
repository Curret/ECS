///@file   world.hpp
///@author Chris Newman
///@brief  Primary interface for the ECS framework.
#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <memory>

#include "array-view.hpp"
#include "typeid.hpp"

#ifdef max
#undef max
#endif

template<typename T, typename... T2>
class TComponentView;

namespace __detail
{
  using ComponentFamily = struct ComponentFamily_;

  // TCompArray /////////////////////////////////////////////////////////////////
  template<typename ID_T>
  struct ICompArrayBase
  {
    virtual void remove(ID_T id)
    {
      (id);
      throw std::runtime_error{ "Not implemented!" };
    }

    virtual bool contains(ID_T id)
    {
      (id);
      throw std::runtime_error{ "Not implemented!" };
    }

    virtual size_t index_of(ID_T ent)
    {
      (ent);
      throw std::runtime_error{ "Not implemented!" };
    }
  };

  ///@brief Type that manages the mappings between components and entities.
  template<typename COMP, typename ID_T>
  class TCompArray : public ICompArrayBase<ID_T>
  {
  public:
    TCompArray();
    ///@brief      Emplaces a component.
    ///@param id   Entity id to assign to.
    ///@param args arguments to construct the component with.
    ///@return     True if successful, false if not.
    template<typename ...ARGS>
    bool insert(ID_T id, ARGS&&... args);
    ///@brief    Removes the component assigned to the given entity.
    ///@param id The entity to remove from.
    void remove(ID_T id) override;
    ///@brief    Gets the component attached to the given entity.
    ///@param id The entity to get from.
    ///@return   The attached component. Returns `nullptr` if none was found.
    COMP* get(ID_T id);
    COMP* get_by_index(size_t i);
    ///@brief       Gets the entity id associated with the given component index.
    ///@param index The component index to get the entity of.
    ///@return      The entity id. Returns value of `Entity::invalid()` if none.
    ID_T get_id(size_t index);
    ///@brief      Gets the entity id associated with the given component.
    ///@param comp Component to get entity of.
    ///@return     The entity id. Returns value of `Entity::invalid()` if none.
    ID_T get_id(COMP& comp);
    ///@brief  Gets the underlying array of components.
    ///@return The underlying array of components.
    std::vector<COMP>& array();
    ///@brief     Checks if the component array contains a component assigned to
    ///           the given entity id.
    ///@param  id The entity id to check against.
    ///@return    True if a component was found, false otherwise.
    bool contains(ID_T id) override;
    ///@brief     Gets the component index of the component attached to the
    ///           given entity.
    ///@param ent Entity ID to check for.
    ///@return    The index into the component array of the attached
    ///           component. Returns numeric_limits<size_t>::max() on failure.
    size_t index_of(ID_T ent) override;
    ///@brief   Gets the number of components currently contained by the component
    ///         array.
    ///@return  Size of component array.
    size_t size() const;
  private:
    std::vector<COMP> arr_;
    std::vector<ID_T> map_;
    std::vector<ID_T> revMap_;
  };

  // TCompRegistry //////////////////////////////////////////////////////////////
  ///@brief Type that manages the association between component types and arrays
  ///       of component objects.
  template<typename ID_T>
  class TCompRegistry
  {
  public:
    ///@brief  Gets the TCompArray object associated with a given component type.
    ///@return Reference to TCompArray object associated with the component type.
    template<typename COMP>
    TCompArray<COMP, ID_T>& get_array();

    ///@brief    Removes all components corresponding to the given id
    ///@param id The id to search for.
    void remove_all_of(ID_T id);
  private:
    std::vector<std::unique_ptr<ICompArrayBase<ID_T> > > reg_;
  };

  // TEntity ////////////////////////////////////////////////////////////////////
  ///@brief Wrapper type that holds the id of a given entity.
  template<typename ID_T>
  class TEntity
  {
  public:
    ID_T id_ = 0;

    TEntity() {}
    TEntity(ID_T id) : id_{id} {}
    ~TEntity() {}

    operator ID_T() { return id_; }

    ///@brief Returns the entity id equivalent of nullptr.
    constexpr static TEntity<ID_T> invalid() { return std::numeric_limits<ID_T>::max(); }
  };

  // TWorld /////////////////////////////////////////////////////////////////////
  ///@brief Helper type used by TWorld.
  template<typename ID_T>
  struct SystemPackage;

  ///@brief Type that wraps the functionality of the ECS system into a single
  ///       interface and provides support for operating on components.
  template<typename ID_T>
  class TWorld
  {
    static_assert(std::is_integral<ID_T>::value, "ID_T must be an integer type.");
    static_assert(std::is_unsigned<ID_T>::value, "ID_T must be unsigned.");

  
  public:
    /// Entity type returned by / passed into functions.
    using Entity = TEntity<ID_T>;

    /// Types of events that system objects can be bound to.
    enum class EventTypes
    {
      none,     ///< Invalid bound type. Never called.
      tick,     ///< Is called on every engine tick. Usually 1/60th of a second.
      tickBegin,///< Is called before `tick`.
      tickEnd   ///< Is called after `tick`.
    };


    // World objects can not be moved or copied.
    TWorld() = default;
    TWorld(TWorld const&) = delete;
    TWorld(TWorld&&) = delete;
    ~TWorld() = default;

    ///@brief Gets the ID of a new entity.
    Entity entity_new() { return lastUsed_++; }

    ///@brief   Removes all components assigned to the given entity.
    ///         `entity_destroy_delayed` should generally be used instead of 
    ///         this function.
    ///@param e The entity to be destroyed.
    void entity_destroy(Entity e);

    ///@brief   Adds the given entity to a list of entities to be removed after
    ///         the completion of the next world tick.
    ///@param e The entity to be destroyed.
    void entity_destroy_delayed(Entity e);

    ///@brief Removes all entities on the removal list and clears it.
    void process_remove();

    ///@brief      Adds the given component type to an entity, constructed
    ///            using the given arguments.
    ///@param e    The entity ID to attach the component to.
    ///@param args The arguments to pass to the component's constructor.
    template<typename COMP, typename ...ARGS>
    void comp_add(Entity e, ARGS&&... args);

    ///@brief   Removes the given component type from the given entity, if
    ///         possible.
    ///@param e The entity ID to remove from.
    template<typename COMP>
    void comp_remove(Entity e);

    ///@brief   Gets the component of the given type from the given component,
    ///         if possible.
    ///@param e The component ID to get from.
    ///@return  Non-owning pointer to the attached component. If it could not
    ///         be found, `nullptr` is returned.
    template<typename COMP>
    COMP* comp_get(Entity e);

    ///@brief  Returns the array of all components of type `COMP`.
    ///@return Array view of all components of type `COMP`.
    template<typename COMP>
    TArrayView<COMP> comp_get();

    ///@brief       Gets the entity that owns component of type `COMP` of
    ///             the given index.
    ///@param index The index of the component being tested.
    ///@return      The entity the component is attached to. If it could not
    ///             be found, value of `Entity::invalid()` is returned
    ///             instead.
    template<typename COMP>
    Entity comp_get_entity(size_t index);

    ///@brief      Gets the entity that owns component of type `COMP` of
    ///            the given component.
    ///@param comp The component being tested.
    ///@return     The entity the component is attached to. If it could not
    ///            be found, value of `Entity::invalid()` is returned
    ///            instead.
    template<typename COMP>
    Entity comp_get_entity(COMP& comp);

    ///@brief      Adds a system to be executed on the given event type.
    ///@param evT  The type of event that fires the system.
    ///@param args Arguments that are forwarded to the system's constructor
    template<typename SYS, typename...CON_ARGS>
    void sys_add(EventTypes evT, CON_ARGS&&...args);

    ///@brief      Executes a given function on each matching set of the
    ///            given component types.
    ///@param func The function to be executed. Should take each of the given
    ///            component types as references.
    template<typename ...COMP, typename FUNC>
    void each(FUNC func);

    ///@brief Calls all systems bound to tick events.
    void tick();

    ///@brief Gets a component view of the specified types.
    template<typename ...COMP>
    TComponentView<ID_T, COMP...> view_get();

  private:
    template<typename T, typename... T2>
    friend class TComponentView;


    template<typename T, typename FUNC>
    void each_single(FUNC func);

    template<typename FUNC, typename T, size_t... IS>
    void each_func_exec(FUNC func, T args, std::index_sequence<IS...>);

    template<typename COMP, typename FROM_COMP>
    COMP* each_get_ptr(size_t i, TCompArray<FROM_COMP, ID_T>& fromCompArr, bool& noNull);

    ID_T                              lastUsed_ = 0;
    TCompRegistry<ID_T>               compReg_;
    std::vector<SystemPackage<ID_T>>  sysTick_;
    std::vector<SystemPackage<ID_T>>  sysTickBegin_;
    std::vector<SystemPackage<ID_T>>  sysTickEnd_;
    std::vector<Entity>               removeList_;
  };

} // namespace __detail

// ALIASES ////////////////////////////////////////////////////////////////////

using IdT = uint32_t;
using World = __detail::TWorld<IdT>;

#include "world-imp.hpp"


// EXAMPLES ///////////////////////////////////////////////////////////////////

namespace {
  inline void example_func() {
    // World object can be instantiated anywhere, but can not be copied or moved.
    World world;
    // World world2 = world; ERROR
    // World world3 = std::move(world); ERROR
    
    // Entities are light weight are easily created with a single function call.
    World::Entity ent = world.entity_new();

    // Components have no data associated with them until assigned. Component 
    // types require no definition before hand, containing arrays are created as 
    // needed. Components can be any type.
    world.comp_add<int32_t>(ent, 1000);
    world.comp_add<float>(ent, -0.5f);

    // The each function is the simplest way of interacting with components.
    // The function iterates over all components that match the given component 
    // types.
    world.each<int32_t, float>([](int32_t& i, float& f) {
      printf("%i, %f\n", i, f);
      i *= f;
    });

    // The primary way of working with components is through Systems.
    // See system.hpp.
  }
}
