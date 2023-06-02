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
    constexpr int absc( lua_State* state )
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
		static constexpr auto step_ratio = 150;
        lua_gc( state, LUA_GCSTEP, step_ratio );
    }

    inline int error_handler( lua_State* state )
    {
        luaL_traceback( state, state, lua_tolstring( state, -1, nullptr ), 0 );
        return 1;
    }

    inline int pcall( lua_State* state, int nargs, int nresults )
    {
        // expected stack layout:
        // - params
        // - chunk

        // determine position of the error handler function
        const auto error_function_pos = size( state ) - nargs;
        lua_pushcclosure( state, error_handler, 0 );

        // move error handler to the top of the stack
        insert( state, error_function_pos );

        // expected stack layout:
        // - params
        // - chunk
        // - error handler

        if ( const auto result = lua_pcall( state, nargs, nresults, error_function_pos ); result == 0 )
                [[likely]]
        {
            remove( state, error_function_pos );
            return nresults == LUA_MULTRET ? size( state ) : nresults;
        }
        else
        {
            switch ( result )
            {
                case LUA_ERRRUN :
                {
                    remove( state, error_function_pos );
                    return -1;
                }
                case LUA_ERRMEM :
                {
                    clear( state );
                    return -2;
                }
                case LUA_ERRERR :
                {
                    clear( state );
                    return -3;
                }
	            default:
				{
		            utilities::assume_unreachable();
				}
            }
        }
    }

    inline int pcall( lua_State* state, int nargs )
    {
        const auto stack_size = size( state );

        // expected stack layout:
        // - params
        // - chunk

        // determine position of the error handler function
        const auto error_function_pos = size( state ) - nargs;
        lua_pushcclosure( state, error_handler, 0 );

        // move error handler to the top of the stack
        insert( state, error_function_pos );

        // expected stack layout:
        // - params
        // - chunk
        // - error handler

        if ( const auto result = lua_pcall( state, nargs, LUA_MULTRET, error_function_pos ); result == 0 )
                [[likely]]
        {
            remove( state, error_function_pos );
            return size( state ) - ( stack_size - 1 - nargs );
        }
        else
        {
            switch ( result )
            {
                case LUA_ERRRUN :
                {
                    remove( state, error_function_pos );
                    return -1;
                }
                case LUA_ERRMEM :
                {
                    clear( state );
                    return -2;
                }
                case LUA_ERRERR :
                {
                    clear( state );
                    return -3;
                }
				default:
				{
					utilities::assume_unreachable();
				}
            }
        }
    }

    inline int pcall( lua_State* state )
    {
        return pcall( state, size( state ) - 1 );
    }

    inline int execute( lua_State* state, const char* name, const char* code, size_t size )
    {
        if ( const auto result = luaL_loadbuffer( state, code, size, name ); result == LUA_OK )
                [[likely]]
        {
            return pcall( state, 0 );
        }
        else
        {
            if ( result == LUA_ERRSYNTAX )
            {
                return -1;
            }

            clear( state );
            return -1;
        }
    }
}