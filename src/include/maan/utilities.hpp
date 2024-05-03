#pragma once

#include <type_traits>

// these definitions come largely from:
// https://github.com/li-script/lightning/blob/master/include/util/common.hpp
// https://github.com/li-script/lightning/blob/master/include/util/typeinfo.hpp

#ifndef __has_builtin
#define __has_builtin(...) 0
#endif
#ifndef __has_attribute
#define __has_attribute(...) 0
#endif
#ifndef __has_cpp_attribute
#define __has_cpp_attribute(...) 0
#endif
#ifndef __has_feature
#define __has_feature(...) 0
#endif
#ifndef __has_include
#define __has_include(...) 0
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(__AMD_64) || defined(_M_AMD64) ||                  \
  defined(_M_IX86) || defined(__i386)
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
#if !MAAN_GNU // NOLINT(*-avoid-unconditional-preprocessor-if)
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
#define MAAN_INLINE __attribute__((always_inline))
#define MAAN_NOINLINE __attribute__((noinline))
#define MAAN_ALIGN(x) __attribute__((aligned(x)))
#define MAAN_TRIVIAL_ABI __attribute__((trivial_abi))
#define MAAN_COLD __attribute__((cold, noinline, disable_tail_calls))
#elif MAAN_MSVC
#define MAAN_INLINE [[msvc::forceinline]]
#define MAAN_NOINLINE __declspec(noinline)
#define MAAN_ALIGN(x) __declspec(align(x))
#define MAAN_TRIVIAL_ABI
#define MAAN_COLD MAAN_NOINLINE
#endif

#ifndef MAAN_STRINGIFY
#define MAAN_STRINGIFY(a) #a
#endif

#ifndef MAAN_STRCAT_I
#define MAAN_STRCAT_I(a, b) a##b
#endif

#ifndef MAAN_STRCAT
#define MAAN_STRCAT(a, b) MAAN_STRCAT_I(a, b)
#endif

namespace maan::utilities {
namespace detail {
struct utype {
  template <typename T>
  operator T() // NOLINT(hicpp-explicit-conversions)
  {}
};

template <typename T>
struct type_namer {
  template <typename id = T>
  static consteval std::string_view id() {
    auto [sig, begin, delta, end] = std::tuple{
#if MAAN_GNU
      std::string_view{__PRETTY_FUNCTION__}, std::string_view{"id"}, +3, "]"
#else
      std::string_view{__FUNCSIG__}, std::string_view{"id"}, +1, ">"
#endif
    };

    // Find the beginning of the name.
    //
    size_t f = sig.size();
    while (sig.substr(--f, begin.size()).compare(begin) != 0) {
      if (f == 0) {
        return "";
      }
    }
    f += begin.size() + delta;

    // Find the end of the string.
    //
    auto l = sig.find_first_of(end, f);
    if (l == std::string::npos) {
      return "";
    }

    // Return the value.
    //
    auto r = sig.substr(f, l - f);
    if (r.size() > 7 && r.substr(0, 7) == "struct ") {
      r.remove_prefix(7);
    }
    if (r.size() > 6 && r.substr(0, 6) == "class ") {
      r.remove_prefix(6);
    }
    return r;
  }

  static constexpr auto name = []() {
    constexpr std::string_view view = type_namer<T>::id<T>();
    std::array<char, view.length() + 1> data = {};
    std::copy(view.begin(), view.end(), data.data());
    return data;
  }();

  consteval operator std::string_view() const // NOLINT(hicpp-explicit-conversions)
  {
    return {&name[0], &name[name.size() - 1]};
  }

  consteval operator const char*() const // NOLINT(hicpp-explicit-conversions)
  {
    return &name[0];
  }
};

template <auto V>
struct value_namer {
  template <auto id = V>
  static consteval std::string_view id() {
    auto [sig, begin, delta, end] = std::tuple{
#if MAAN_GNU
      std::string_view{__PRETTY_FUNCTION__}, std::string_view{"id"}, +3, ']'
#else
      std::string_view{__FUNCSIG__}, std::string_view{"id"}, +0, '>'
#endif
    };

    // Find the beginning of the name.
    //
    size_t f = sig.rfind(begin);
    if (f == std::string::npos) {
      return "";
    }
    f += begin.size() + delta;

    // Find the end of the string.
    //
    auto l = sig.find(end, f);
    if (l == std::string::npos) {
      return "";
    }

    // Return the value.
    //
    return sig.substr(f, l - f);
  }

  static constexpr auto name = []() {
    constexpr std::string_view view = value_namer<V>::id<V>();
    std::array<char, view.length() + 1> data = {};
    std::copy(view.begin(), view.end(), data.data());
    return data;
  }();

  consteval operator std::string_view() const {
    return {&name[0], &name[name.size() - 1]};
  }

  consteval operator const char*() const {
    return &name[0];
  }
};

static consteval uint32_t fnv1a32_hash(const char* sig) {
  uint32_t tmp = 0x811c9dc5;
  while (*sig) // NOLINT(readability-implicit-bool-conversion)
  {
    tmp ^= *sig++; // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    tmp *= 0x01000193;
  }
  return tmp;
}
} // namespace detail

template <typename T>
struct type_tag {
  using type = T;

  template <typename t = T>
  static consteval uint32_t hash() {
    return std::integral_constant<uint32_t, detail::fnv1a32_hash(FUNCTION_NAME)>{};
  }

  template <auto C = 0>
  static consteval std::string_view to_string() {
    return detail::type_namer<T>{};
  }

  template <auto C = 0>
  static consteval const char* c_str() {
    return detail::type_namer<T>{};
  }
};

template <auto V>
struct const_tag {
  using value_type = decltype(V);
  static constexpr value_type value = V;

  constexpr operator value_type() const noexcept {
    return value;
  }

  template <auto v = V>
  static consteval uint32_t hash() {
    return std::integral_constant<uint32_t, detail::fnv1a32_hash(FUNCTION_NAME)>{};
  }

  template <auto C = 0>
  static consteval std::string_view to_string() {
    return detail::value_namer<V>{};
  }

  template <auto C = 0>
  static consteval const char* c_str() {
    return detail::value_namer<V>{};
  }
};

class string_literal {
  const char* v;
  size_t length;

public:
  template <size_t Size>
  constexpr string_literal(char (&literal)[Size]) noexcept : v{literal}, length{Size - 1} {}

  template <size_t Size>
  constexpr string_literal(const char (&literal)[Size]) noexcept : v{literal}, length{Size - 1} {}

  [[nodiscard]] constexpr size_t size() const noexcept {
    return length;
  }

  [[nodiscard]] constexpr const char* data() const noexcept {
    return v;
  }

  constexpr operator const char*() const noexcept {
    return v;
  }

  constexpr operator std::string_view() const noexcept {
    return {v, length};
  }
};

template <typename T>
concept aggregate_member_countable = requires(T) { requires std::is_aggregate_v<T>; };

template <aggregate_member_countable T>
consteval int aggregate_member_counter(auto&&... members) {
  if constexpr (!requires { T{members...}; }) {
    return sizeof...(members) - 1;
  } else {
    return aggregate_member_counter<T>(members..., detail::utype{});
  }
}

template <typename T>
consteval int member_count() {
  using type = std::remove_cvref_t<T>;

  if constexpr (!std::is_class_v<type>) {
    return 0;
  } else if constexpr (aggregate_member_countable<type>) {
    return aggregate_member_counter<type>();
  }

  return -1;
}

static constexpr auto maximum_member_countable_member_count = 10;

template <typename T>
concept member_countable = requires(T) { requires 0 <= member_count<T>() && maximum_member_countable_member_count >= member_count<T>(); };

MAAN_INLINE constexpr decltype(auto) visit_members_types(member_countable auto&& object, auto&& visitor) {
  using type = std::remove_cvref_t<decltype(object)>();
  constexpr auto count = member_count<decltype(object)>();

  if constexpr (count == 0) {
    return visitor.template operator()<>();
  } else if constexpr (count == 1) {
    auto&& [a1] = object;
    return visitor.template operator()<decltype(a1)>();
  } else if constexpr (count == 2) {
    auto&& [a1, a2] = object;
    return visitor.template operator()<decltype(a1), decltype(a2)>();
  } else if constexpr (count == 3) {
    auto&& [a1, a2, a3] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3)>();
  } else if constexpr (count == 4) {
    auto&& [a1, a2, a3, a4] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4)>();
  } else if constexpr (count == 5) {
    auto&& [a1, a2, a3, a4, a5] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5)>();
  } else if constexpr (count == 6) {
    auto&& [a1, a2, a3, a4, a5, a6] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6)>();
  } else if constexpr (count == 7) {
    auto&& [a1, a2, a3, a4, a5, a6, a7] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7)>();
  } else if constexpr (count == 8) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8] = object;
    return visitor
      .template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8)>();
  } else if constexpr (count == 9) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = object;
    return visitor.template
    operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9)>();
  } else if constexpr (count == 10) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = object;
    return visitor.template operator()<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8),
                                       decltype(a9), decltype(a10)>();
  } else {
    static_assert(std::is_same_v<type, void>, "type can not be used for counting members");
  }
}

MAAN_INLINE constexpr decltype(auto) visit_members(member_countable auto&& object, auto&& visitor) {
  constexpr auto count = member_count<decltype(object)>();

  if constexpr (count == 0) {
    return visitor();
  } else if constexpr (count == 1) {
    auto&& [a1] = object;
    return visitor(a1);
  } else if constexpr (count == 2) {
    auto&& [a1, a2] = object;
    return visitor(a1, a2);
  } else if constexpr (count == 3) {
    auto&& [a1, a2, a3] = object;
    return visitor(a1, a2, a3);
  } else if constexpr (count == 4) {
    auto&& [a1, a2, a3, a4] = object;
    return visitor(a1, a2, a3, a4);
  } else if constexpr (count == 5) {
    auto&& [a1, a2, a3, a4, a5] = object;
    return visitor(a1, a2, a3, a4, a5);
  } else if constexpr (count == 6) {
    auto&& [a1, a2, a3, a4, a5, a6] = object;
    return visitor(a1, a2, a3, a4, a5, a6);
  } else if constexpr (count == 7) {
    auto&& [a1, a2, a3, a4, a5, a6, a7] = object;
    return visitor(a1, a2, a3, a4, a5, a6, a7);
  } else if constexpr (count == 8) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8] = object;
    return visitor(a1, a2, a3, a4, a5, a6, a7, a8);
  } else if constexpr (count == 9) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = object;
    return visitor(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  } else if constexpr (count == 10) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = object;
    return visitor(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  } else {
    static_assert(std::is_same_v<decltype(object), void>, "type can not be used for counting members");
  }
}

MAAN_INLINE constexpr auto to_underlying(auto&& enum_value) {
  using type = std::remove_cvref_t<decltype(enum_value)>;
  return static_cast<std::underlying_type_t<type>>(enum_value);
}

MAAN_INLINE static inline void breakpoint() {
#if __has_builtin(__builtin_debugtrap)
  __builtin_debugtrap();
#elif MAAN_MSVC
  __debugbreak();
#endif
}

MAAN_INLINE static inline void assume_that(bool condition) {
#if __has_builtin(__builtin_assume)
  __builtin_assume(condition);
#elif MAAN_MSVC
  __assume(condition);
#endif
}

[[noreturn]] MAAN_INLINE static inline void assume_unreachable() {
#if __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#else
  assume_that(false);
#endif
}
} // namespace maan::utilities
