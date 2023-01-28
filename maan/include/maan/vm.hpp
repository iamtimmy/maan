#pragma once

#include <utility>
#include <new>

#include <lua.hpp>
#include <maan/stack.hpp>
#include <maan/function.hpp>

namespace maan
{
    class vm
    {
        lua_State* state;

    public:
        [[nodiscard]] bool running() const
        {
            return state != nullptr;
        }

        [[nodiscard]] size_t working_set() const
        {
            return operations::working_set( state );
        }

        [[nodiscard]] lua_State* get_state() const
        {
            return state;
        }

        [[nodiscard]] size_t stack_size() const
        {
            return operations::size( state );
        }

        void pop( int n = 1 )
        {
            operations::pop( state, n );
        }

        template< typename _type >
        [[nodiscard]] _type get( int index ) const
        {
            return stack::get< _type >( state, index );
        }

        template< typename _type >
        [[nodiscard]] bool is( int index ) const
        {
            return stack::is< _type >( state, index );
        }

        template< typename _type >
        void push( _type value ) const
        {
            if constexpr ( function::is_function< _type > )
            {
                return function::push( state, value );
            }

            return stack::push< _type >( state, value );
        }

        template< int nresults, typename... _types >
        int call( _types ... args )
        {
            constexpr int param_count = sizeof...( _types );
            if constexpr ( param_count != 0 )
            {
                (push( args ), ...);
            }

            if constexpr ( nresults == LUA_MULTRET )
            {
                return operations::pcall( state, param_count );
            }

            return operations::pcall( state, param_count, nresults );
        }

        vm()
                : state{ luaL_newstate() }
        {
            if ( state == nullptr )
            {
                throw std::bad_alloc();
            }

            luaL_openlibs( state );
        }

        ~vm()
        {
            if ( state == nullptr )
            { return; }

            lua_close( state );
        }

        vm( vm&& other ) noexcept
                : state{ std::exchange( other.state, nullptr ) }
        {
        };

        vm& operator=( vm&& other ) noexcept
        {
            if ( this != &other )
            {
                state = std::exchange( other.state, nullptr );
            }

            return *this;
        }

        vm( vm const& ) = delete;
        vm& operator=( vm const& ) = delete;
    };
}