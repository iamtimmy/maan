#pragma once

#include <type_traits>

#include <lua.hpp>
#include <maan/vm_type.hpp>
#include <maan/utilities.hpp>

namespace maan::operations
{
    inline int size( lua_State* state )
    {
        return lua_gettop( state );
    }

    template< int index >
    int absc( lua_State* state )
    {
        if constexpr ( index > LUA_REGISTRYINDEX && index < 0 )
        {
            return size( state ) + index + 1;
        }
        else if constexpr ( index > 0 )
        {
            return index;
        }
        else
        {
            static_assert( "bad maan::stack::absc index" );
        }
    }

    inline int abs( lua_State* state, int index )
    {
        if ( index > LUA_REGISTRYINDEX && index < 0 )
        {
            return size( state ) + index + 1;
        }

        return index;
    }

    inline void pop( lua_State* state, int index )
    {
        lua_settop( state, -index - 1 );
    }

    inline void clear( lua_State* state )
    {
        lua_settop( state, 0 );
    }

    inline void remove( lua_State* state, int index )
    {
        lua_remove( state, index );
    }

    inline void insert( lua_State* state, int index )
    {
        lua_insert( state, index );
    }

    inline vm_type type( lua_State* state, int index )
    {
        return static_cast<vm_type>(lua_type( state, index ));
    }

    inline bool is( lua_State* state, int index, vm_type type )
    {
        return lua_type( state, index ) == utilities::to_underlying( type );
    }

    inline size_t working_set( lua_State* state )
    {
        return lua_gc( state, LUA_GCCOUNT, 0 );
    }

    inline void stop_gc( lua_State* state )
    {
        lua_gc( state, LUA_GCSTOP, 0 );
    }

    inline void start_gc( lua_State* state )
    {
        lua_gc( state, LUA_GCRESTART, 0 );
    }

    inline void perform_gc_cycle( lua_State* state )
    {
        lua_gc( state, LUA_GCCOLLECT, 0 );
    }

    inline void perform_gc_step( lua_State* state )
    {
        lua_gc( state, LUA_GCSTEP, 150 );
    }
}