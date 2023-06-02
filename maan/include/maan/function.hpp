#pragma once

#include <tuple>

#include <lua.hpp>
#include <maan/stack.hpp>

namespace maan::function
{
	template< typename return_type, typename... types >
	struct function_info;

	template< typename return_type, typename... types >
	struct function_info< return_type( types ... ) >
	{
		using cvreturn_type = std::remove_cvref_t< return_type >;
		using tuple_type = std::tuple< types... >;

		template< int index >
		using argument_types = std::tuple_element_t< index, std::tuple< types... > >;

		static consteval size_t check_return()
		{
			static_assert( stack::is_lua_pushable< cvreturn_type >, "wrapped function has unsupported return type" );
			return 1;
		}

		template< size_t argument_number = 0 >
		static consteval size_t check_and_count_arguments()
		{
			if constexpr ( argument_number == sizeof...( types ) )
			{
				return argument_number;
			}
			else
			{
				using arg_type = std::remove_cvref_t< std::tuple_element_t< argument_number, tuple_type > >;
				static_assert( stack::is_lua_pushable< arg_type >, "wrapped function has unsupported argument type" );

				return check_and_count_arguments< argument_number + 1 >();
			}
		}

		static constexpr auto return_count = check_return();
		static constexpr auto argument_count = check_and_count_arguments();

		template< size_t index = 0 >
		static void set_tuple( lua_State* state, tuple_type& tuple )
		{
			if constexpr ( index == sizeof...( types ) )
			{
			}
			else
			{
				using arg_type = std::remove_cvref_t< argument_types< index > >;

				if ( stack::is< arg_type >( state, index + 1 ) )
						[[likely]]
				{
					std::get< index >( tuple ) = stack::get< arg_type >( state, index + 1 );
					return set_tuple< index + 1 >( state, tuple );
				}
				else [[unlikely]]
				{
					luaL_error( state, "invalid argument %d { got: %s | expected: %s }",
					            index,
					            luaL_typename( state, index + 1 ),
					            stack::name< arg_type >( state, index + 1 ) );
					utilities::assume_unreachable();
				}
			}
		}
	};

	template< typename return_type, typename... types >
	struct function_info< return_type( * )( types ... ) >
			: function_info< return_type( types ... ) >
	{
	};

	template< typename return_type, typename... types >
	struct function_info< return_type( * const )( types ... ) >
			: function_info< return_type( types ... ) >
	{
	};

	template< typename clazz, typename return_type, typename... types >
	struct function_info< return_type( clazz::* )( types ... ) >
			: function_info< return_type( clazz*, types ... ) >
	{
	};

	template< typename clazz, typename return_type, typename... types >
	struct function_info< return_type( clazz::* )( types ... ) const >
			: function_info< return_type( clazz*, types ... ) >
	{
	};

	template< typename type >
	concept is_function =
	requires( type )
	{
		requires std::is_function_v< std::remove_pointer_t< std::remove_cvref_t< type > > >;
	};

	void push( lua_State* state, is_function auto&& function )
	{
		using info = function_info< std::remove_cvref_t< decltype( function ) > >;

		struct call_info
		{
			std::remove_cvref_t< decltype( function ) > ptr;
		};

		static constexpr auto call_info_size = sizeof( call_info );
		new( lua_newuserdata( state, call_info_size ) ) call_info{ function };

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

			const auto call = static_cast<call_info* >(lua_touserdata( state, lua_upvalueindex( 1 ) ));

			if constexpr ( std::is_same_v< typename info::cvreturn_type, void > )
			{
				std::apply( call->ptr, std::move( params ) );
				return 0;
			}
			else
			{
				stack::push( state, std::apply( call->ptr, std::move( params ) ) );
				return 1;
			}
		};

		lua_pushcclosure( state, call_wrapper, 1 );
	}
}