#pragma once

#include <lua.hpp>

#include <concepts>

#include <maan/utilities.hpp>
#include <maan/operations.hpp>

namespace maan::stack
{
	namespace detail
	{
		struct lua_userdata
		{
			uintptr_t hash;
			void* data;
		};

		template< typename _type >
		concept is_lua_integer_convertable =
		requires( _type )
		{
			requires std::is_integral_v< _type > &&
			         ( std::is_signed_v< _type > && sizeof( _type ) < sizeof( int64_t ) ||
			           std::is_unsigned_v< _type > && sizeof( _type ) < sizeof( uint32_t ) );
		};

		template< typename _type >
		concept is_lua_number_convertable =
		requires( _type )
		{
			requires std::is_floating_point_v< _type > ||
			         ( std::is_signed_v< _type > && sizeof( _type ) >= sizeof( int64_t ) ||
			           std::is_unsigned_v< _type > && sizeof( _type ) >= sizeof( uint32_t ) );
		};

		template< typename _type >
		concept is_lua_string_convertable =
		requires( _type )
		{
			requires std::is_same_v< _type, const char* > || std::is_same_v< _type, char* > ||
			         std::is_same_v< _type, std::string > || std::is_same_v< _type, std::string_view > ||
			         std::is_nothrow_convertible_v< _type, utilities::string_literal >;
		};

		template< typename _type >
		concept is_lua_pushable_pointer =
		requires( _type )
		{
			std::is_pointer_v< _type > && std::is_class_v< std::remove_pointer< _type > >;
		};

		template< typename _type >
		concept is_lua_fundamental_convertable =
		requires( _type )
		{
			requires std::is_same_v< bool, _type > ||
			         is_lua_integer_convertable< _type > ||
			         is_lua_number_convertable< _type > ||
			         is_lua_string_convertable< _type >;
		};
	}

	template< typename _type >
	concept is_lua_pushable =
	requires( _type )
	{
		requires std::is_same_v< void, _type > || // can always push nothing onto the stack
		         detail::is_lua_fundamental_convertable< _type > || // fundamental cpp and lua types
		         detail::is_lua_fundamental_convertable< std::remove_pointer< _type > > ||
		         detail::is_lua_pushable_pointer< _type >;
	};

	MAAN_INLINE void push( lua_State* state, auto&& object )
	requires is_lua_pushable< std::remove_cvref_t< decltype( object )>>
	{
		using type = std::remove_cvref_t< decltype( object ) >;
		using type_noptr = std::remove_pointer_t< type >;

		if constexpr ( std::is_same_v< void, decltype( object ) > )
		{
			return;
		}
		else if constexpr ( detail::is_lua_fundamental_convertable< type > )
		{
			if constexpr ( std::is_same_v< bool, type > )
			{
				lua_pushboolean( state, std::forward< decltype( object ) >( object ) );
			}
			else if constexpr ( detail::is_lua_integer_convertable< type > )
			{
				lua_pushinteger( state, std::forward< decltype( object ) >( object ) );
			}
			else if constexpr ( detail::is_lua_number_convertable< type > )
			{
				lua_pushnumber( state, std::forward< decltype( object ) >( object ) );
			}
			else if constexpr ( detail::is_lua_string_convertable< type > )
			{
				if constexpr ( std::is_nothrow_convertible_v< type, utilities::string_literal > )
				{
					const auto literal = utilities::string_literal{ object };
					lua_pushlstring( state, literal.data(), literal.size() );
				}
				else if constexpr ( std::is_same_v< std::string, type > || std::is_same_v< std::string_view, type > )
				{
					lua_pushlstring( state, object.data(), object.size() );
				}
				else
				{
					lua_pushstring( state, std::forward< decltype( object ) >( object ) );
				}
			}
		}
		else if constexpr ( std::is_pointer_v< type > && std::is_class_v< type_noptr > )
		{
			const auto type_hash = static_cast<std::uintptr_t>(utilities::type_tag< type >::hash());
			new( lua_newuserdata( state, sizeof( type_hash ) + sizeof( void* ) ) ) detail::lua_userdata{ type_hash, reinterpret_cast<void*>(object) };
		}
		else
		{
			static_assert( std::is_same_v< void, type >, "unsupported type to stack::push" );
			utilities::assume_unreachable();
		}
	}

	template< typename return_type >
	MAAN_INLINE decltype( auto ) get( lua_State* state, int&& index )
	requires is_lua_pushable< std::remove_cvref_t< return_type > >
	{
		using type = std::remove_cvref_t< return_type >;
		using type_noptr = std::remove_pointer_t< type >;

		if constexpr ( std::is_same_v< void, type > )
		{
			return;
		}
		else if constexpr ( detail::is_lua_fundamental_convertable< type > )
		{
			if constexpr ( std::is_same_v< bool, type > )
			{
				return static_cast<bool>(lua_toboolean( state, std::forward< int&& >( index ) ));
			}
			else if constexpr ( detail::is_lua_integer_convertable< type > )
			{
				return static_cast<type>(lua_tointeger( state, std::forward< int&& >( index ) ));
			}
			else if constexpr ( detail::is_lua_number_convertable< type > )
			{
				return static_cast<type>(lua_tonumber( state, std::forward< int&& >( index ) ));
			}
			else if constexpr ( detail::is_lua_string_convertable< type > )
			{
				if constexpr ( std::is_same_v< type, std::string > || std::is_same_v< type, std::string_view > )
				{
					size_t size;
					const auto data = lua_tolstring( state, std::forward< int&& >( index ), &size );
					return type{ data, size };
				}
				else
				{
					return static_cast<type>(lua_tolstring( state, std::forward< int&& >( index ), nullptr ));
				}
			}
		}
		else if constexpr ( std::is_pointer_v< type > && std::is_class_v< type_noptr > )
		{
			const auto data = static_cast<detail::lua_userdata*>(lua_touserdata( state, index ));
			return reinterpret_cast<type>(data->data);
		}
		else
		{
			static_assert( std::is_same_v< void, type >, "unsupported type to stack::push" );
			utilities::assume_unreachable();
		}
	}

	template< typename checked_type >
	MAAN_INLINE bool is( lua_State* state, int&& index )
	requires is_lua_pushable< std::remove_cvref_t< checked_type > >
	{
		using type = std::remove_cvref_t< checked_type >;
		using type_noptr = std::remove_pointer_t< type >;

		if constexpr ( std::is_same_v< void, type > )
		{
			const auto type = operations::type( state, index );
			return type == vm_type::none || type == vm_type::nil;
		}
		else if constexpr ( detail::is_lua_fundamental_convertable< type > )
		{
			if constexpr ( std::is_same_v< bool, type > )
			{
				return operations::is( state, index, vm_type::boolean );
			}
			else if constexpr ( detail::is_lua_integer_convertable< type > )
			{
				return operations::is( state, index, vm_type::number );
			}
			else if constexpr ( detail::is_lua_number_convertable< type > )
			{
				return operations::is( state, index, vm_type::number );
			}
			else if constexpr ( detail::is_lua_string_convertable< type > )
			{
				return operations::is( state, index, vm_type::string );
			}
		}
		else if constexpr ( std::is_pointer_v< type > && std::is_class_v< type_noptr > )
		{
			const auto data = static_cast<detail::lua_userdata*>(lua_touserdata( state, index ));
			return data->hash == static_cast<std::uintptr_t>(utilities::type_tag< type >::hash());
		}
		else
		{
			static_assert( std::is_same_v< void, type >, "unsupported type to stack::push" );
			utilities::assume_unreachable();
		}
	}

	template< typename checked_type >
	MAAN_INLINE std::string_view name( [[maybe_unused]] lua_State* state, [[maybe_unused]] int&& index )
	{
		using type = std::remove_cvref_t< checked_type >;
		using type_noptr = std::remove_pointer_t< type >;

		if constexpr ( std::is_same_v< void, type > )
		{
			return "nil";
		}
		else if constexpr ( detail::is_lua_fundamental_convertable< type > )
		{
			if constexpr ( std::is_same_v< bool, type > )
			{
				return "boolean";
			}
			else if constexpr ( detail::is_lua_integer_convertable< type > )
			{
				return "integer";
			}
			else if constexpr ( detail::is_lua_number_convertable< type > )
			{
				return "number";
			}
			else if constexpr ( detail::is_lua_string_convertable< type > )
			{
				return "string";
			}
		}
		else if constexpr ( std::is_class_v< type > && utilities::member_countable< type > )
		{
			return utilities::type_tag< type >::to_string();
		}
		else if constexpr ( std::is_pointer_v< type > && std::is_class_v< type_noptr > )
		{
			return utilities::type_tag< type >::to_string();
		}
		else
		{
			static_assert( std::is_same_v< void, type >, "unsupported type to stack::push" );
			utilities::assume_unreachable();
		}
	}
}