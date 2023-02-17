#include <catch2/catch_all.hpp>

#include <maan.hpp>

const auto code = R"(
return function(a)
    return a + 100
end
)";

TEST_CASE( "basic code", "[code]" )
{
    auto vm = maan::vm();
    REQUIRE( vm.running() == true );

    REQUIRE( vm.stack_size() == 0 );

    REQUIRE( vm.execute( "basic code", code ) == 1 );
    REQUIRE( vm.stack_size() == 1 );

    REQUIRE( vm.call( 100 ) == 1 );

    REQUIRE( vm.is< int >( -1 ) == true );
    REQUIRE( vm.get< int >( -1 ) == 200 );
}