#pragma once

#include <utility>
#include <string_view>

#include <lua.hpp>
#include <maan/operations.hpp>

namespace maan::stl
{
    template< typename _type >
    struct stl_type;

    template< typename _type >
    concept is_stl_type =
    requires( _type )
    {
        {
        stl_type< _type >::push( nullptr, std::declval< _type >())
        } -> std::same_as< void >;

        {
        stl_type< _type >::get( nullptr, std::declval< int >())
        } -> std::same_as< _type >;

        {
        stl_type< _type >::is( nullptr, std::declval< int >())
        } -> std::same_as< bool >;

        {
        stl_type< _type >::name
        } -> std::same_as< std::string_view >;
    };


}