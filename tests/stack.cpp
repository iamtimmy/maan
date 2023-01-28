#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE( "basic stack", "[stack][maan]" )
{
    auto vm = maan::vm();
    REQUIRE( vm.running() == true );

    REQUIRE( vm.stack_size() == 0 );

    {
        vm.push( true );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< bool >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< bool >( -1 );
        REQUIRE( value == true );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }

    {
        vm.push( nullptr );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< nullptr_t >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< nullptr_t >( -1 );
        REQUIRE( value == nullptr );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }

    {
        vm.push( 100 );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< int >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< int >( -1 );
        REQUIRE( value == 100 );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }

    {
        vm.push( 100. );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< double >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< double >( -1 );
        REQUIRE( value == 100. );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }

    {
        vm.push( 100.f );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< float >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< float >( -1 );
        REQUIRE( value == 100.f );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }

    {
        const char* str = "very interesting string";

        vm.push( str );
        REQUIRE( vm.stack_size() == 1 );

        REQUIRE( vm.is< const char* >( -1 ) == true );
        REQUIRE( vm.is< char* >( -1 ) == true );
        REQUIRE( vm.stack_size() == 1 );

        const auto value = vm.get< const char* >( -1 );
        REQUIRE( std::strcmp( value, str ) == 0 );
        REQUIRE( vm.stack_size() == 1 );

        vm.pop();
        REQUIRE( vm.stack_size() == 0 );
    }
}