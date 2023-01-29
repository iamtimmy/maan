#include <catch2/catch_all.hpp>

#include <maan.hpp>

const auto syntax_error_code = R"(
return -;
)";

TEST_CASE( "syntax error code", "[syntax error code][maan]" )
{
    auto vm = maan::vm();
    REQUIRE( vm.running() == true );

    REQUIRE( vm.stack_size() == 0 );

    REQUIRE( vm.execute( "code", syntax_error_code ) == -1 );
    REQUIRE( vm.stack_size() == 1 );

    REQUIRE( vm.is< const char* >( -1 ) == true );
    INFO( vm.get< const char* >( -1 ) );
}

const auto error_code = R"(
return non_existant_function();
)";

TEST_CASE( "error code", "[error code][maan]" )
{
    auto vm = maan::vm();
    REQUIRE( vm.running() == true );

    REQUIRE( vm.stack_size() == 0 );

    REQUIRE( vm.execute( "code", error_code ) == -1 );
    REQUIRE( vm.stack_size() == 1 );

    REQUIRE( vm.is< const char* >( -1 ) == true );
    INFO( vm.get< const char* >( -1 ) );
}
