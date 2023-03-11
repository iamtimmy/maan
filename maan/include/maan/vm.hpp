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

		int execute( std::string_view name, std::string_view code ) const
		{
			return operations::execute( state, name.data(), code.data(), code.size() );
		}

		template< typename type >
		[[nodiscard]] decltype( auto ) get( int&& index ) const
		{
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( std::is_class_v< cvtype > && utilities::member_countable< cvtype > )
			{
				static constexpr auto count = utilities::member_count< cvtype >();

				const auto stack_start_index = operations::abs( state, index );

				const auto fn = [this, stack_start_index]< typename... types >()
				{
					std::tuple< types... > member_values;
					aggregate::set_tuple( state, stack_start_index, member_values );

					const auto fn = []( auto&& ... params ) -> cvtype
					{
						return cvtype{ params... };
					};

					return std::apply( fn, member_values );
				};

				return utilities::visit_members_types < type > ( type{}, fn );
			}
			else
			{
				return stack::get< type >( state, std::forward< int >( index ) );
			}
		}

		template< typename type >
		[[nodiscard]] bool is( int&& index ) const
		{
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( std::is_class_v< cvtype > && utilities::member_countable< cvtype > )
			{
				static constexpr auto count = utilities::member_count< cvtype >();

				const auto stack_size = operations::size( state );
				const auto start_index = operations::abs( state, index );
				const auto stop_index = start_index + count - 1;

				if ( stop_index > stack_size )
					return false;

				const auto fn = [this, start_index]< typename... types >()
				{
					return aggregate::check< 0, types... >( state, start_index );
				};

				return utilities::visit_members_types < type > ( type{}, fn );
			}
			else
			{
				return stack::is< type >( state, std::forward< int >( index ) );
			}
		}

		void push( auto&& value ) const
		{
			using type = decltype( value );
			using cvtype = std::remove_cvref_t< type >;

			if constexpr ( std::is_class_v< cvtype > && utilities::member_countable< cvtype > )
			{
				utilities::visit_members( value, [this]( auto&& ... members )
				{
					(stack::push( state, members ), ...);
				} );
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