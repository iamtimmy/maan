#pragma once

#include <tuple>

#include <lua.hpp>
#include <maan/stack.hpp>
#include <maan/aggregate.hpp>

namespace maan::function
{
	struct function_requirements
	{
		size_t argument_count;
		size_t stack_slot_count;
	};

	template< typename return_type, typename... types >
	struct function_info;

	template< typename return_type, typename... types >
	struct function_info< return_type( types ... ) >
	{
		using cvreturn_type = std::remove_cvref_t< return_type >;
		using tuple_type = std::tuple< types... >;

		template< int index >
		using argument_types = std::tuple_element_t< index, std::tuple< types... > >;

		template< size_t index = 0, size_t result = 0 >
		static consteval function_requirements check()
		{
			if constexpr ( index == sizeof...( types ) )
			{
				return { index, result };
			}
			else
			{
				using arg_type = std::remove_cvref_t< std::tuple_element_t< index, tuple_type > >;
				static_assert( stack::is_lua_pushable< arg_type > || aggregate::is_lua_convertable< arg_type >,
				               "wrapped function has unsupported argument type" );

				if constexpr ( aggregate::is_lua_convertable< arg_type > )
				{
					return check< index + 1, result + aggregate::stack_size< arg_type >() >();
				}
				else
				{
					return check< index + 1, result + 1 >();
				}
			}
		}

		static constexpr auto requirements = check();

		template< size_t tuple_index = 0, size_t lua_index = 0 >
		static void set_tuple( lua_State* state, tuple_type& tuple )
		{
			if constexpr ( tuple_index == sizeof...( types ) )
			{
			}
			else
			{
				using arg_type = std::remove_cvref_t< argument_types< tuple_index > >;

				if constexpr ( aggregate::is_lua_convertable< arg_type > )
				{
					if ( aggregate::is< arg_type >( state, lua_index + 1 ) )
							[[likely]]
					{
						std::get< tuple_index >( tuple ) = aggregate::get< arg_type >( state, lua_index + 1 );
						return set_tuple< tuple_index + 1, lua_index + aggregate::stack_size< arg_type >() >( state, tuple );
					}
				}
				else
				{
					if ( stack::is< arg_type >( state, lua_index + 1 ) )
							[[likely]]
					{
						std::get< tuple_index >( tuple ) = stack::get< arg_type >( state, lua_index + 1 );
						return set_tuple< tuple_index + 1, lua_index + 1 >( state, tuple );
					}
				}

				luaL_error( state, "invalid argument %d { got: %s | expected: %s }",
				            lua_index,
				            luaL_typename( state, lua_index + 1 ),
				            stack::name< arg_type >( state, lua_index + 1 ) );
				utilities::assume_unreachable();
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

	static void push( lua_State* state, is_function auto&& function )
	{
		using info = function_info< std::remove_cvref_t< decltype( function ) > >;

		static_assert( stack::is_lua_pushable< typename info::cvreturn_type > ||
		               aggregate::is_lua_convertable< typename info::cvreturn_type >,
		               "wrapped function has unsupported return type" );

		struct call_info
		{
			std::remove_cvref_t< decltype( function ) > ptr;
		};

		static constexpr auto call_info_size = sizeof( call_info );
		new( lua_newuserdata( state, call_info_size ) ) call_info{ function };

		static lua_CFunction const call_wrapper = +[]( lua_State* state ) -> int
		{
			static constexpr auto requirements = info::requirements;

			if ( const auto stack_size = operations::size( state ); requirements.stack_slot_count != stack_size )
			{
				luaL_error( state, "invalid arguments { expected: %d | stack_size: %d }", requirements.stack_slot_count, stack_size );
				utilities::assume_unreachable();
			}

			using tuple = typename info::tuple_type;

			tuple params;
			info::set_tuple( state, params );

			const auto* call = static_cast<call_info* >(lua_touserdata( state, lua_upvalueindex( 1 ) ));

			if constexpr ( std::is_same_v< typename info::cvreturn_type, void > )
			{
				std::apply( call->ptr, std::move( params ) );
				return 0;
			}
			else
			{
				if constexpr ( aggregate::is_lua_convertable< typename info::cvreturn_type > )
				{
					aggregate::push( state, std::apply( call->ptr, std::move( params ) ) );
					return aggregate::stack_size< typename info::cvreturn_type >();
				}
				else
				{
					stack::push( state, std::apply( call->ptr, std::move( params ) ) );
					return 1;
				}
			}
		};

		lua_pushcclosure( state, call_wrapper, 1 );
	}
}