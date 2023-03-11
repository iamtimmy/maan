#pragma once

#include <tuple>

namespace maan::aggregate
{
	template< size_t index = 0, typename... types >
	static void set_tuple( lua_State* state, int stack_index, std::tuple< types... >& tuple )
	{
		if constexpr ( index == sizeof...( types ) )
		{
			return;
		}
		else
		{
			using argument_type = std::tuple_element_t< index, std::tuple< types... > >;

			std::get< index >( tuple ) = stack::get< argument_type >( state, stack_index + index );
			return set_tuple< index + 1 >( state, stack_index, tuple );
		}
	}

	template< size_t index = 0, typename... types >
	static bool check( lua_State* state, int stack_index )
	{
		if constexpr ( index == sizeof...( types ) )
		{
			return true;
		}
		else
		{
			using argument_type = std::tuple_element_t< index, std::tuple< types... > >;

			if ( !stack::is< argument_type >( state, stack_index + index ) )
				return false;
			else
				return check< index + 1, types... >( state, stack_index );
		}
	}
}