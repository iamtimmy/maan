#pragma once

#include <type_traits>

// these definitions come largely from:
// https://github.com/li-script/lightning/blob/master/include/util/common.hpp
// https://github.com/li-script/lightning/blob/master/include/util/typeinfo.hpp

#ifndef __has_builtin
#define __has_builtin( ... ) 0
#endif
#ifndef __has_attribute
#define __has_attribute( ... ) 0
#endif
#ifndef __has_cpp_attribute
#define __has_cpp_attribute(...) 0
#endif
#ifndef __has_feature
#define __has_feature( ... ) 0
#endif
#ifndef __has_include
#define __has_include(...) 0
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(__AMD_64) || defined(_M_AMD64) || defined(_M_IX86) || defined(__i386)
#ifndef MAAN_X86_32
#if UINTPTR_MAX == 0xFFFFFFFF
#define MAAN_X86_32 1
#define MAAN_X86_64 0
#else
#define MAAN_X86_32 0
#define MAAN_X86_64 1
#endif
#endif
#else
#error "unsupported architecture"
#endif

#if defined(__GNUC__) || defined(__clang__)
#define MAAN_GNU 1
#else
#define MAAN_GNU 0
#endif
#ifdef _MSC_VER
#define MAAN_MS_EXTS 1
#if !MAAN_GNU
#define MAAN_MSVC 1
#endif
#endif

#ifndef MAAN_DEBUG
#if defined(NDEBUG) && !defined(_DEBUG)
#define MAAN_DEBUG 0
#else
#define MAAN_DEBUG 1
#endif
#endif

#if MAAN_MS_EXTS
#define FUNCTION_NAME __FUNCSIG__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#if MAAN_GNU
#define MAAN_INLINE      __attribute__((always_inline))
#define MAAN_NOINLINE    __attribute__((noinline))
#define MAAN_ALIGN(x)    __attribute__((aligned(x)))
#define MAAN_TRIVIAL_ABI __attribute__((trivial_abi))
#define MAAN_COLD        __attribute__((cold, noinline, disable_tail_calls))
#elif MAAN_MSVC
#define MAAN_INLINE      [[msvc::forceinline]]
#define MAAN_NOINLINE    __declspec(noinline)
#define MAAN_ALIGN( x )    __declspec(align(x))
#define MAAN_TRIVIAL_ABI
#define MAAN_COLD        MAAN_NOINLINE
#endif

#ifndef MAAN_STRINGIFY
#define MAAN_STRINGIFY( a ) \
    #a
#endif

#ifndef MAAN_STRCAT_I
#define MAAN_STRCAT_I( a, b ) \
    a##b
#endif

#ifndef MAAN_STRCAT
#define MAAN_STRCAT( a, b ) \
    MAAN_STRCAT_I(a, b)
#endif

namespace maan::utilities
{
    namespace detail
    {
        template< typename T >
        struct type_namer
        {
            template< typename __id__ = T >
            static consteval std::string_view _id__()
            {
                auto [sig, begin, delta, end] = std::tuple{
#if MAAN_GNU
                        std::string_view{__PRETTY_FUNCTION__}, std::string_view{"_id__"}, +3, "]"
#else
                        std::string_view{ __FUNCSIG__ }, std::string_view{ "_id__" }, +1, ">"
#endif
                };

                // Find the beginning of the name.
                //
                size_t f = sig.size();
                while ( sig.substr( --f, begin.size() ).compare( begin ) != 0 )
                    if ( f == 0 )
                        return "";
                f += begin.size() + delta;

                // Find the end of the string.
                //
                auto l = sig.find_first_of( end, f );
                if ( l == std::string::npos )
                    return "";

                // Return the value.
                //
                auto r = sig.substr( f, l - f );
                if ( r.size() > 7 && r.substr( 0, 7 ) == "struct " )
                {
                    r.remove_prefix( 7 );
                }
                if ( r.size() > 6 && r.substr( 0, 6 ) == "class " )
                {
                    r.remove_prefix( 6 );
                }
                return r;
            }

            static constexpr auto name = []()
            {
                constexpr std::string_view view = type_namer< T >::_id__< T >();
                std::array< char, view.length() + 1 > data = {};
                std::copy( view.begin(), view.end(), data.data() );
                return data;
            }();

            inline consteval operator std::string_view() const
            {
                return { &name[0], &name[name.size() - 1] };
            }

            inline consteval operator const char*() const
            {
                return &name[0];
            }
        };

        template< auto V >
        struct value_namer
        {
            template< auto __id__ = V >
            static consteval std::string_view _id__()
            {
                auto [sig, begin, delta, end] = std::tuple{
#if MAAN_GNU
                        std::string_view{__PRETTY_FUNCTION__}, std::string_view{"_id__"}, +3, ']'
#else
                        std::string_view{ __FUNCSIG__ }, std::string_view{ "_id__" }, +0, '>'
#endif
                };

                // Find the beginning of the name.
                //
                size_t f = sig.rfind( begin );
                if ( f == std::string::npos )
                    return "";
                f += begin.size() + delta;

                // Find the end of the string.
                //
                auto l = sig.find( end, f );
                if ( l == std::string::npos )
                    return "";

                // Return the value.
                //
                return sig.substr( f, l - f );
            }

            static constexpr auto name = []()
            {
                constexpr std::string_view view = value_namer< V >::_id__< V >();
                std::array< char, view.length() + 1 > data = {};
                std::copy( view.begin(), view.end(), data.data() );
                return data;
            }();

            inline consteval operator std::string_view() const
            {
                return { &name[0], &name[name.size() - 1] };
            }

            inline consteval operator const char*() const
            {
                return &name[0];
            }
        };

        static consteval uint32_t fnv1a32_hash( const char* sig )
        {
            uint32_t tmp = 0x811c9dc5;
            while ( *sig )
            {
                tmp ^= *sig++;
                tmp *= 0x01000193;
            }
            return tmp;
        }
    };

    template< typename T >
    struct type_tag
    {
        using type = T;

        template< typename t = T >
        static consteval uint32_t hash()
        {
            return std::integral_constant< uint32_t, detail::fnv1a32_hash( FUNCTION_NAME ) >{};
        }

        template< auto C = 0 >
        static consteval std::string_view to_string()
        {
            return detail::type_namer< T >{};
        }

        template< auto C = 0 >
        static consteval const char* c_str()
        {
            return detail::type_namer< T >{};
        }
    };

    template< auto V >
    struct const_tag
    {
        using value_type = decltype( V );
        static constexpr value_type value = V;

        constexpr operator value_type() const noexcept
        {
            return value;
        }

        template< auto v = V >
        static consteval uint32_t hash()
        {
            return std::integral_constant< uint32_t, detail::fnv1a32_hash( FUNCTION_NAME ) >{};
        }

        template< auto C = 0 >
        static consteval std::string_view to_string()
        {
            return detail::value_namer< V >{};
        }

        template< auto C = 0 >
        static consteval const char* c_str()
        {
            return detail::value_namer< V >{};
        }
    };

    auto to_underlying( auto enum_value )
    {
        return static_cast<std::underlying_type_t< decltype( enum_value ) >>( enum_value );
    }

    MAAN_INLINE static inline void breakpoint()
    {
#if __has_builtin( __builtin_debugtrap )
        __builtin_debugtrap();
#elif MAAN_MSVC
        __debugbreak();
#endif
    }

    MAAN_INLINE static inline void assume_that( bool condition )
    {
#if __has_builtin( __builtin_assume )
        __builtin_assume( condition );
#elif MAAN_MSVC
        __assume( condition );
#endif
    }

    [[noreturn]] MAAN_INLINE static inline void assume_unreachable()
    {
#if __has_builtin( __builtin_unreachable )
        __builtin_unreachable();
#else
        assume_that( false );
#endif
    }
}