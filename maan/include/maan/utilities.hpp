#pragma once

#include <type_traits>

// these definitions come largely from:
// https://github.com/li-script/lightning/blob/545bb63830d2685d923d2e7cfd176dc0bc17296b/include/util/common.hpp
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