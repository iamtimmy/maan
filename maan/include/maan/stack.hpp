#pragma once

#include <lua.hpp>
#include <maan/operations.hpp>
#include <maan/builtin.hpp>

namespace maan::stack
{
	template< typename _type >
	concept is_pushable_type =
	requires( _type )
	{
		requires builtin::is_builtin_type< _type > ||
		         std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer_t< _type>>;
	};

	template< typename _type >
	consteval bool is_pushable()
	{
		if constexpr ( is_pushable_type< _type > )
		{
			return true;
		}

		return false;
	}

	struct userdata
	{
		uintptr_t hash;
		void* data;
	};

	template< typename _type >
	bool is( lua_State* state, int index )
	{
		if constexpr ( builtin::is_builtin_type< _type > )
		{
			return builtin::builtin_type< _type >::is( state, index );
		}
		else if constexpr ( std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer_t< _type>> )
		{
			const auto type_hash = static_cast<std::uintptr_t>(utilities::type_tag< _type >::hash());
			const auto data = static_cast<userdata*>(lua_touserdata( state, index ));
			return data->hash == type_hash;
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
		else if constexpr ( std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer_t< _type>> )
		{
			const auto data = static_cast<userdata*>(lua_touserdata( state, index ));
			return reinterpret_cast<_type>(data->data);
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
		else if constexpr ( std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer_t< _type>> )
		{
			const auto type_hash = static_cast<std::uintptr_t>(utilities::type_tag< _type >::hash());
			new( lua_newuserdata( state, sizeof( type_hash ) + sizeof( void* ) ) ) userdata{ type_hash, reinterpret_cast<void*>(value) };
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
		else if constexpr ( std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer_t< _type>> )
		{
			return utilities::type_tag< _type >::to_string();
		}
		else
		{
			static_assert( "maan::stack::name unsupported type" );
		}

		utilities::assume_unreachable();
	}
}