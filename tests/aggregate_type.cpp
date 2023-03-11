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
