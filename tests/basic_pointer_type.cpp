#include <catch2/catch_all.hpp>

#include <maan.hpp>

struct type
{
	float a;
	float b;
};

TEST_CASE( "basic pointer type", "[types]" )
{
	auto vm = maan::vm();
	REQUIRE( vm.running() == true );

	REQUIRE( vm.stack_size() == 0 );

	auto data = type{ 100.f, 100.f };

	vm.push( +[]( type* a )
	{
		return a;
	} );

	REQUIRE( vm.stack_size() == 1 );

	REQUIRE( vm.call( &data ) == 1 );

	REQUIRE( vm.stack_size() == 1 );

	REQUIRE( vm.is< type* >( -1 ) == true );
	auto transfered_data = vm.get< type* >( -1 );

	REQUIRE( transfered_data != nullptr );

	auto result = transfered_data->a + transfered_data->b;
	REQUIRE( result == 200.f );
}
