#pragma once

#include <tuple>

#include <lua.hpp>
#include <maan/stack.hpp>

namespace maan::function
{
    template< typename _return, typename... _types >
    struct function_info;

    template< typename _return, typename... _types >
    struct function_info< _return( _types ... ) >
    {
        using return_type = _return;
        using tuple_type = std::tuple< _types... >;

        template< int index >
        using argument_types = std::tuple_element_t< index, std::tuple< _types... > >;

        static consteval size_t check_return()
        {
            static_assert( stack::is_pushable< return_type >(), "wrapped function has unsupported return type" );
            return 1;
        }

        template< size_t _argument_number = 0, size_t _return_value = 0 >
        static consteval size_t check_and_count_arguments()
        {
            if constexpr ( _argument_number == sizeof...( _types ) )
            {
                return _return_value;
            }
            else
            {
                using arg_type = std::tuple_element_t< _argument_number, tuple_type >;
                static_assert( stack::is_pushable< arg_type >(), "wrapped function has unsupported argument type" );

                return check_and_count_arguments< _argument_number + 1, _return_value + 1 >();
            }
        }

        static constexpr auto return_count = check_return();
        static constexpr auto argument_count = check_and_count_arguments();

        template< size_t index = 0 >
        static void set_tuple( lua_State* l, tuple_type& tuple )
        {
            if constexpr ( index == sizeof...( _types ) )
            {
            }
            else
            {
                using arg_type = argument_types< index >;

                if ( stack::is< arg_type >( l, index + 1 ) )
                        [[likely]]
                {
                    std::get< index >( tuple ) = stack::get< arg_type >( l, index + 1 );
                    return set_tuple< index + 1 >( l, tuple );
                }
                else [[unlikely]]
                {
                    luaL_error( l, "invalid argument %d { got: %s | expected: %s }",
                                index,
                                luaL_typename( l, index + 1 ),
                                get_name< arg_type >( l, index + 1 ) );
                    utilities::assume_unreachable();
                }
            }
        }
    };

    template< typename _return, typename... _types >
    struct function_info< _return( * )( _types ... ) >
            : function_info< _return( _types ... ) >
    {
    };

    template< typename _return, typename... _types >
    struct function_info< _return( * const )( _types ... ) >
            : function_info< _return( _types ... ) >
    {
    };

    template< typename _class, typename _return, typename... _types >
    struct function_info< _return( _class::* )( _types ... ) >
            : function_info< _return( _class*, _types ... ) >
    {
    };

    template< typename _class, typename _return, typename... _types >
    struct function_info< _return( _class::* )( _types ... ) const >
            : function_info< _return( _class*, _types ... ) >
    {
    };

    template< typename _type >
    concept is_function =
    requires( _type )
    {
        requires std::is_function_v< _type >;
    };

    template< is_function _type >
    void push_function( lua_State* state, _type function )
    {
        using info = function_info< _type >;

        struct call_info
        {
            _type ptr;
        };

        static constexpr auto call_info_size = sizeof( call_info );
        new( lua_newuserdata( state, call_info_size ) ) call_info( function );

        static lua_CFunction const call_wrapper = +[]( lua_State* state ) -> int
        {
            if ( const auto stack_size = operations::size( state ); stack_size != info::argument_count )
                    [[unlikely]]
            {
                luaL_error( state, "invalid arguments { expected: %d | stack_size: %d }", info::argument_count, stack_size );
                utilities::assume_unreachable();
            }

            using tuple = typename info::tuple_type;

            tuple params;
            info::set_tuple( state, params );

            const auto info = static_cast<call_info* >(lua_touserdata( state, lua_upvalueindex( 1 ) ));

            if constexpr ( std::is_same_v< typename info::return_type, void > )
            {
                std::apply( info->ptr, std::move( params ) );
                return 0;
            }
            else
            {
                stack::push( state, std::apply( info->ptr, std::move( params ) ) );
                return 1;
            }
        };

        lua_pushcclosure( state, call_wrapper, 1 );
    }
}