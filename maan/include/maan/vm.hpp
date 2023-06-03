#pragma once

#include <utility>
#include <new>

#include <lua.hpp>
#include <maan/stack.hpp>
#include <maan/function.hpp>
#include <maan/aggregate.hpp>

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

		[[nodiscard]] int execute( std::string_view name, std::string_view code ) const
		{
			return operations::execute( state, name.data(), code.data(), code.size() );
		}

		template< typename type >
		[[nodiscard]] decltype( auto ) get( int index ) const
		{
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( aggregate::is_lua_convertable< cvtype > )
			{
				return aggregate::get< type >( state, index );
			}
			else
			{
				return stack::get< type >( state, index );
			}
		}

		template< typename type >
		[[nodiscard]] bool is( int index ) const
		{
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( aggregate::is_lua_convertable< cvtype > )
			{
				return aggregate::is< type >( state, index );
			}
			else
			{
				return stack::is< type >( state, index );
			}
		}

		void push( auto&& value ) const
		{
			using type = decltype( value );
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( aggregate::is_lua_convertable< cvtype > )
			{
				return aggregate::push( state, std::forward< type >( value ) );
			}
			else if constexpr ( function::is_function< type > )
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
			using info = function::function_info< void( types... ) >;

			constexpr int param_count = sizeof...( types );
			if constexpr ( param_count != 0 )
			{
				(push( std::forward< types >( args ) ), ...);
			}

			if constexpr ( nresults == LUA_MULTRET )
			{
				return operations::pcall( state, info::requirements.stack_slot_count );
			}
			else
			{
				return operations::pcall( state, info::requirements.stack_slot_count, nresults );
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