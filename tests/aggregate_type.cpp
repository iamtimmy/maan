#include <catch2/catch_all.hpp>

#include <maan.hpp>

struct vec2
{
	float a;
	float b;
};

struct other
{
	const char* a;
};

TEST_CASE( "aggregate type", "[types]" )
{
	auto vm = maan::vm();
	REQUIRE( vm.running() == true );

	REQUIRE( vm.stack_size() == 0 );

	vm.push( vec2{ 100.f, 100.f } );

	REQUIRE( vm.stack_size() == 2 );

	REQUIRE( vm.get< float >( -1 ) == 100.f );
	REQUIRE( vm.get< float >( -2 ) == 100.f );

	REQUIRE( vm.is< vec2 >( -2 ) == true );
	REQUIRE( vm.is< other >( -2 ) == false );

	auto v = vm.get< vec2 >( -2 );
	REQUIRE( v.a == 100.f );
	REQUIRE( v.b == 100.f );
}

TEST_CASE( "aggregate types in functions", "[types]" )
{
	auto vm = maan::vm();

	vm.push( +[]( vec2 a ) -> vec2
	{
		return a;
	} );

	REQUIRE( vm.stack_size() == 1 );

	const auto result = vm.call( vec2{ 100.f, 100.f } );
	if ( result == -1 )
	{
		CATCH_ERROR( vm.get< std::string >( -1 ) );
	}

	REQUIRE( vm.stack_size() == 2 );
	REQUIRE( vm.get< float >( -1 ) == 100.f );
	REQUIRE( vm.get< float >( -2 ) == 100.f );
}