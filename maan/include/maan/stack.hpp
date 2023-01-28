#pragma once

#include <lua.hpp>
#include <maan/operations.hpp>
#include <maan/builtin.hpp>

namespace maan::stack
{
    template< typename _type >
    consteval bool is_pushable()
    {
        if constexpr ( builtin::is_builtin_type< _type > )
        {
            return true;
        }

        return false;
    }

    template< typename _type >
    bool is( lua_State* state, int index )
    {
        if constexpr ( builtin::is_builtin_type< _type > )
        {
            return builtin::builtin_type< _type >::is( state, index );
        }
        else
        {
            static_assert( "maan::stack::is unsupported type" );
        }

        utilities::assume_unreachable();
    }

    template< typename _type >
    _type get( lua_State* state, int index )
    {
        if constexpr ( builtin::is_builtin_type< _type > )
        {
            return builtin::builtin_type< _type >::get( state, index );
        }
        else
        {
            static_assert( "maan::stack::get unsupported type" );
        }

        utilities::assume_unreachable();
    }

    template< typename _type >
    void push( lua_State* state, _type value )
    {
        if constexpr ( builtin::is_builtin_type< _type > )
        {
            return builtin::builtin_type< _type >::push( state, value );
        }
        else
        {
            static_assert( "maan::stack::push unsupported type" );
        }

        utilities::assume_unreachable();
    }

    template< typename _type >
    std::string_view name( [[maybe_unused]] lua_State* state, [[maybe_unused]] int index )
    {
        if constexpr ( builtin::is_builtin_type< _type > )
        {
            return builtin::builtin_type< _type >::name;
        }
        else
        {
            static_assert( "maan::stack::name unsupported type" );
        }

        utilities::assume_unreachable();
    }
}