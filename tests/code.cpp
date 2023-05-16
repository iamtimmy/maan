#include <catch2/catch_all.hpp>

#include <maan.hpp>

const auto number_code = R"(
return function(a)
    return a + 100
end
)";

const auto string_code = R"(
return function(a)
    return a .. " concatenation"
end
)";

TEST_CASE( "code", "[code]" )
{
	auto vm = maan::vm();
	REQUIRE( vm.running() == true );

	{
		REQUIRE( vm.stack_size() == 0 );

		REQUIRE( vm.execute( "number code", number_code ) == 1 );
		REQUIRE( vm.stack_size() == 1 );

		REQUIRE( vm.call( 100 ) == 1 );

		REQUIRE( vm.is< int >( -1 ) == true );
		REQUIRE( vm.get< int >( -1 ) == 200 );

		vm.pop();
		REQUIRE( vm.stack_size() == 0 );
	}

	{
		REQUIRE( vm.stack_size() == 0 );

		REQUIRE( vm.execute( "string code", string_code ) == 1 );
		REQUIRE( vm.stack_size() == 1 );

		REQUIRE( vm.call( "string" ) == 1 );

		REQUIRE( vm.is< std::string >( -1 ) == true );
		REQUIRE( vm.get< std::string >( -1 ) == "string concatenation" );

		vm.pop();
		REQUIRE( vm.stack_size() == 0 );
	}
}