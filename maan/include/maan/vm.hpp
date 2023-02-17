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
		void release()
		{
			state = nullptr;
		}

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

		void pop( int n = 1 ) const
		{
			operations::pop( state, n );
		}

		int execute( std::string_view name, std::string_view code )
		{
			return operations::execute( state, name.data(), code.data(), code.size() );
		}

		template< typename type >
		[[nodiscard]] decltype( auto ) get( int&& index ) const
		{
			return stack::get< type >( state, std::forward< int&& >( index ) );
		}

		template< typename type >
		[[nodiscard]] bool is( int&& index ) const
		{
			return stack::is< type >( state, std::forward< int&& >( index ) );
		}

		void push( auto&& value ) const
		{
			using type = decltype( value );

			if constexpr ( function::is_function< type > )
			{
				return function::push( state, std::forward< type >( value ) );
			}
			else
			{
				return stack::push( state, std::forward< type >( value ) );
			}
		}

		template< int nresults = LUA_MULTRET, typename... types >
		int call( types&& ... args ) const
		{
			constexpr int param_count = sizeof...( types );
			if constexpr ( param_count != 0 )
			{
				(push( std::forward< types >( args ) ), ...);
			}

			if constexpr ( nresults == LUA_MULTRET )
			{
				return operations::pcall( state, param_count );
			}
			else
			{
				return operations::pcall( state, param_count, nresults );
			}
		}

		vm()
				: state{ luaL_newstate() }
		{
			if ( state != nullptr )
			{
				luaL_openlibs( state );
			}
		}

		explicit vm( lua_State* state )
				: state{ state }
		{
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