#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE( "basic type checking", "[types]" )
{
	auto vm = maan::vm();
	REQUIRE( vm.running() == true );

	{
		vm.push( true );
		REQUIRE( vm.stack_size() == 1 );

		REQUIRE( vm.is< bool >( -1 ) == true );
		REQUIRE( vm.is< int >( -1 ) == false );
		REQUIRE( vm.is< long >( -1 ) == false );
		REQUIRE( vm.is< long long >( -1 ) == false );
		REQUIRE( vm.is< float >( -1 ) == false );
		REQUIRE( vm.is< double >( -1 ) == false );
		REQUIRE( vm.is< const char* >( -1 ) == false );

		vm.pop();
		REQUIRE( vm.stack_size() == 0 );
	}

	{
		const char* test = "test";

		vm.push( test );
		REQUIRE( vm.stack_size() == 1 );

		REQUIRE( vm.is< bool >( -1 ) == false );
		REQUIRE( vm.is< int >( -1 ) == false );
		REQUIRE( vm.is< long >( -1 ) == false );
		REQUIRE( vm.is< long long >( -1 ) == false );
		REQUIRE( vm.is< float >( -1 ) == false );
		REQUIRE( vm.is< double >( -1 ) == false );
		REQUIRE( vm.is< const char* >( -1 ) == true );

		vm.pop();
		REQUIRE( vm.stack_size() == 0 );
	}
}