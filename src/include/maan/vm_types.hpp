#pragma once

#include <lua.hpp>

#include <concepts>

#include <maan/operatons.hpp>
#include <maan/utilities.hpp>

namespace maan::vm_type {
namespace detail {
struct lua_userdata {
  uintptr_t hash;
  void* data;
};

template <typename T>
concept is_lua_integer_convertable = requires(T) {
  requires std::is_integral_v<T> && (std::is_signed_v<T> && sizeof(T) < sizeof(int64_t) || std::is_unsigned_v<T> && sizeof(T) < sizeof(uint32_t));
};

template <typename T>
concept is_lua_number_convertable = requires(T) {
  requires std::is_floating_point_v<T> ||
             (std::is_signed_v<T> && sizeof(T) >= sizeof(int64_t) || std::is_unsigned_v<T> && sizeof(T) >= sizeof(uint32_t));
};

template <typename T>
concept is_lua_string_convertable = requires(T) {
  requires std::is_same_v<T, const char*> || std::is_same_v<T, char*> || std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
             std::is_nothrow_convertible_v<T, utilities::string_literal>;
};

template <typename T>
concept is_lua_pushable_pointer = requires(T) { std::is_pointer_v<T>&& std::is_class_v<std::remove_pointer<T>>; };

template <typename T>
concept is_lua_fundamental_convertable =
  requires(T) { requires std::is_same_v<bool, T> || is_lua_integer_convertable<T> || is_lua_number_convertable<T> || is_lua_string_convertable<T>; };
} // namespace detail

template <typename T>
concept is_lua_pushable = requires(T) {
  requires std::is_void_v<T> ||                           // can always push nothing onto the vm_type
             detail::is_lua_fundamental_convertable<T> || // fundamental cpp
                                                          // and lua types
             detail::is_lua_fundamental_convertable<std::remove_pointer<T>> || detail::is_lua_pushable_pointer<T>;
};

MAAN_INLINE void push(lua_State* state, auto&& object)
  requires is_lua_pushable<std::remove_cvref_t<decltype(object)>>
{
  using type = std::remove_cvref_t<decltype(object)>;
  using type_noptr = std::remove_pointer_t<type>;

  if constexpr (std::is_same_v<void, decltype(object)>) {
    return;
  } else if constexpr (detail::is_lua_fundamental_convertable<type>) {
    if constexpr (std::is_same_v<bool, type>) {
      lua_pushboolean(state, std::forward<decltype(object)>(object));
    } else if constexpr (detail::is_lua_integer_convertable<type>) {
      lua_pushinteger(state, std::forward<decltype(object)>(object));
    } else if constexpr (detail::is_lua_number_convertable<type>) {
      lua_pushnumber(state, std::forward<decltype(object)>(object));
    } else if constexpr (detail::is_lua_string_convertable<type>) {
      if constexpr (std::is_nothrow_convertible_v<type, utilities::string_literal>) {
        const auto literal = utilities::string_literal{object};
        lua_pushlstring(state, literal.data(), literal.size());
      } else if constexpr (std::is_same_v<std::string, type> || std::is_same_v<std::string_view, type>) {
        lua_pushlstring(state, object.data(), object.size());
      } else {
        lua_pushstring(state, std::forward<decltype(object)>(object));
      }
    }
  } else if constexpr (std::is_pointer_v<type> && std::is_class_v<type_noptr>) {
    const auto type_hash = static_cast<std::uintptr_t>(utilities::type_tag<type>::hash());
    new (lua_newuserdata(state, sizeof(type_hash) + sizeof(void*))) detail::lua_userdata{type_hash, reinterpret_cast<void*>(object)};
  } else {
    static_assert(std::is_same_v<void, type>, "unsupported type to vm_type::push");
    utilities::assume_unreachable();
  }
}

template <typename return_type>
MAAN_INLINE decltype(auto) get(lua_State* state, int index)
  requires is_lua_pushable<std::remove_cvref_t<return_type>>
{
  using type = std::remove_cvref_t<return_type>;
  using type_noptr = std::remove_pointer_t<type>;

  if constexpr (std::is_same_v<void, type>) {
    return;
  } else if constexpr (detail::is_lua_fundamental_convertable<type>) {
    if constexpr (std::is_same_v<bool, type>) {
      return static_cast<bool>(lua_toboolean(state, index));
    } else if constexpr (detail::is_lua_integer_convertable<type>) {
      return static_cast<type>(lua_tointeger(state, index));
    } else if constexpr (detail::is_lua_number_convertable<type>) {
      return static_cast<type>(lua_tonumber(state, index));
    } else if constexpr (detail::is_lua_string_convertable<type>) {
      if constexpr (std::is_same_v<type, std::string> || std::is_same_v<type, std::string_view>) {
        size_t size{};
        const auto* data = lua_tolstring(state, index, &size);
        return type{data, size};
      } else {
        return static_cast<type>(lua_tolstring(state, index, nullptr));
      }
    }
  } else if constexpr (std::is_pointer_v<type> && std::is_class_v<type_noptr>) {
    const auto* data = static_cast<detail::lua_userdata*>(lua_touserdata(state, index));
    return reinterpret_cast<type>(data->data);
  } else {
    static_assert(std::is_same_v<void, type>, "unsupported type to vm_type::push");
    utilities::assume_unreachable();
  }
}

template <typename checked_type>
MAAN_INLINE bool is(lua_State* state, int index)
  requires is_lua_pushable<std::remove_cvref_t<checked_type>>
{
  using type = std::remove_cvref_t<checked_type>;
  using type_noptr = std::remove_pointer_t<type>;

  if constexpr (std::is_same_v<void, type>) {
    const auto type = operations::type(state, index);
    return type == vm_type_tag::none || type == vm_type_tag::nil;
  } else if constexpr (detail::is_lua_fundamental_convertable<type>) {
    if constexpr (std::is_same_v<bool, type>) {
      return operations::is(state, index, vm_type_tag::boolean);
    } else if constexpr (detail::is_lua_integer_convertable<type> || detail::is_lua_number_convertable<type>) {
      return operations::is(state, index, vm_type_tag::number);
    } else if constexpr (detail::is_lua_string_convertable<type>) {
      return operations::is(state, index, vm_type_tag::string);
    }
  } else if constexpr (std::is_pointer_v<type> && std::is_class_v<type_noptr>) {
    const auto* data = static_cast<detail::lua_userdata*>(lua_touserdata(state, index));
    return data->hash == static_cast<std::uintptr_t>(utilities::type_tag<type>::hash());
  } else {
    static_assert(std::is_same_v<void, type>, "unsupported type to vm_type::push");
    utilities::assume_unreachable();
  }
}

template <typename checked_type>
MAAN_INLINE std::string_view name([[maybe_unused]] lua_State* state, [[maybe_unused]] int index) {
  using type = std::remove_cvref_t<checked_type>;
  using type_noptr = std::remove_pointer_t<type>;

  if constexpr (std::is_same_v<void, type>) {
    return "nil";
  } else if constexpr (detail::is_lua_fundamental_convertable<type>) {
    if constexpr (std::is_same_v<bool, type>) {
      return "boolean";
    } else if constexpr (detail::is_lua_integer_convertable<type>) {
      return "integer";
    } else if constexpr (detail::is_lua_number_convertable<type>) {
      return "number";
    } else if constexpr (detail::is_lua_string_convertable<type>) {
      return "string";
    }
  } else if constexpr (std::is_class_v<type> && utilities::member_countable<type>) {
    return utilities::type_tag<type>::to_string();
  } else if constexpr (std::is_pointer_v<type> && std::is_class_v<type_noptr>) {
    return utilities::type_tag<type>::to_string();
  } else {
    static_assert(std::is_same_v<void, type>, "unsupported type to vm_type::push");
    utilities::assume_unreachable();
  }
}
} // namespace maan::vm_type