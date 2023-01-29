#pragma once

#include <utility>
#include <string_view>

#include <lua.hpp>
#include <maan/operations.hpp>

namespace maan::builtin
{
	template< typename _type >
	struct builtin_type;

	template< typename _type >
	concept is_builtin_type =
	requires( _type )
	{
		{
		builtin_type< _type >::push( nullptr, std::declval< _type >() )
		} -> std::same_as< void >;

		{
		builtin_type< _type >::get( nullptr, std::declval< int >() )
		} -> std::same_as< _type >;

		{
		builtin_type< _type >::is( nullptr, std::declval< int >() )
		} -> std::same_as< bool >;

		{
		builtin_type< _type >::name
		} -> std::convertible_to< std::string_view >;
	};

	template< typename _type >
	concept is_lua_integral_type =
	requires( _type )
	{
		requires std::is_integral_v< _type > &&
		         ( std::is_signed_v< _type > && sizeof( _type ) < sizeof( int64_t ) ||
		           std::is_unsigned_v< _type > && sizeof( _type ) < sizeof( uint32_t ) );
	};

	template< typename _type >
	concept is_lua_number_type =
	requires( _type )
	{
		requires std::is_floating_point_v< _type > ||
		         ( std::is_signed_v< _type > && sizeof( _type ) >= sizeof( int64_t ) ||
		           std::is_unsigned_v< _type > && sizeof( _type ) >= sizeof( uint32_t ) );
	};

	template<>
	struct builtin_type< bool >
	{
		static void push( lua_State* state, bool value )
		{
			lua_pushboolean( state, static_cast<int>(value) );
		}

		static bool get( lua_State* state, int index )
		{
			return static_cast<bool>(lua_toboolean( state, index ));
		}

		static bool is( lua_State* state, int index )
		{
			const auto type = operations::type( state, index );
			return type == vm_type::boolean || type == vm_type::nil;
		}

		static constexpr std::string_view name = "boolean";
	};

	template<>
	struct builtin_type< std::nullptr_t >
	{
		static void push( lua_State* state, [[maybe_unused]] std::nullptr_t value )
		{
			lua_pushnil( state );
		}

		static std::nullptr_t get( [[maybe_unused]] lua_State* state, [[maybe_unused]] int index )
		{
			return nullptr;
		}

		static bool is( lua_State* state, int index )
		{
			return operations::is( state, index, vm_type::nil );
		}

		static constexpr std::string_view name = "nil";
	};

	template< is_lua_integral_type _type >
	struct builtin_type< _type >
	{
		static void push( lua_State* state, _type value )
		{
			lua_pushinteger( state, static_cast< lua_Integer >( value ) );
		}

		static _type get( lua_State* state, int index )
		{
			return static_cast< _type >( lua_tointeger( state, index ));
		}

		static bool is( lua_State* state, int index )
		{
			return operations::is( state, index, vm_type::number );
		}

		static constexpr std::string_view name = "integral_number";
	};

	template< is_lua_number_type _type >
	struct builtin_type< _type >
	{
		static void push( lua_State* state, _type value )
		{
			lua_pushnumber( state, static_cast< lua_Number >( value ) );
		}

		static _type get( lua_State* state, int index )
		{
			return static_cast< _type >( lua_tonumber( state, index ));
		}

		static bool is( lua_State* state, int index )
		{
			return operations::is( state, index, vm_type::number );
		}

		static constexpr std::string_view name = "real_number";
	};

	template< typename _type >
	requires std::_Is_any_of_v< _type, char*, const char* >
	struct builtin_type< _type >
	{
		static void push( lua_State* state, _type value )
		{
			lua_pushstring( state, value );
		}

		static _type get( lua_State* state, int index )
		{
			return lua_tolstring( state, index, nullptr );
		}

		static bool is( lua_State* state, int index )
		{
			return operations::is( state, index, vm_type::string );
		}

		static constexpr std::string_view name = "string";
	};
}