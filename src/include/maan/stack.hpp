#pragma once

#include <maan/utilities.hpp>
#include <maan/operations.hpp>
#include <maan/vm_types.hpp>
#include <maan/aggregate.hpp>

namespace maan::stack {
template <typename T>
[[nodiscard]] MAAN_INLINE bool is(lua_State* state, int const index) {
  using type = std::remove_cvref_t<T>;

  if constexpr (aggregate::is_lua_convertable<type>) {
    return aggregate::is<T>(state, index);
  } else {
    return vm_types::is<T>(state, index);
  }
}

template <typename T>
MAAN_INLINE void push(lua_State* state, T&& value) {
  using type = std::remove_cvref_t<T>;

  if constexpr (aggregate::is_lua_convertable<type>) {
    return aggregate::push(state, std::forward<T>(value));
  } else {
    return vm_types::push(state, std::forward<T>(value));
  }
}

template <typename T>
[[nodiscard]] MAAN_INLINE decltype(auto) get(lua_State* state, int const index) {
  using type = std::remove_cvref_t<T>;

  if constexpr (aggregate::is_lua_convertable<type>) {
    return aggregate::get<T>(state, index);
  } else {
    return vm_types::get<T>(state, index);
  }
}

template <int result_count = LUA_MULTRET, typename... Ts>
[[nodiscard]] MAAN_INLINE int call(lua_State* state, Ts&&... args) {
  constexpr auto stack_slot_count = []<size_t index = 0, size_t result = 0>(this auto&& self) {
    using tuple_type = std::tuple<Ts...>;

    if constexpr (index == sizeof...(Ts)) {
      return result;
    } else {
      using arg_type = std::remove_cvref_t<std::tuple_element_t<index, tuple_type>>;
      static_assert(!std::is_function_v<std::remove_pointer_t<arg_type>>, "c++ function types and lambdas cannot be pushed onto the stack directly");
      static_assert(vm_types::is_lua_convertable<arg_type> || aggregate::is_lua_convertable<arg_type>, "unknown argument type");

      if constexpr (aggregate::is_lua_convertable<arg_type>) {
        return self.template operator()<index + 1, result + aggregate::stack_size<arg_type>()>();
      } else {
        return self.template operator()<index + 1, result + 1>();
      }
    }
  }();

  if constexpr (constexpr int param_count = sizeof...(Ts); param_count != 0) {
    (push(state, std::forward<Ts>(args)), ...);
  }

  if constexpr (result_count == LUA_MULTRET) {
    return operations::pcall(state, stack_slot_count);
  } else {
    return operations::pcall(state, stack_slot_count, result_count);
  }
}
} // namespace maan::stack