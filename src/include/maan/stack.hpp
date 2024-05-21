#pragma once

#include <maan/utilities.hpp>
#include <maan/operatons.hpp>
#include <maan/vm_types.hpp>
#include <maan/aggregate.hpp>

namespace maan::stack {
template <typename type>
[[nodiscard]] MAAN_INLINE bool is(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::is<type>(state, index);
  } else {
    return vm_types::is<type>(state, index);
  }
}

template <typename type>
MAAN_INLINE void push(lua_State* state, type&& value) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::push(state, std::forward<type>(value));
  } else {
    return vm_types::push(state, std::forward<type>(value));
  }
}

template <typename type>
[[nodiscard]] MAAN_INLINE decltype(auto) get(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::get<type>(state, index);
  } else {
    return vm_types::get<type>(state, index);
  }
}

template <int result_count = LUA_MULTRET, typename... types>
[[nodiscard]] MAAN_INLINE int call(lua_State* state, types&&... args) {
  constexpr auto stack_slot_count = []<size_t index = 0, size_t result = 0>(this auto&& self) {
    using tuple_type = std::tuple<types...>;

    if constexpr (index == sizeof...(types)) {
      return result;
    } else {
      using arg_type = std::remove_cvref_t<std::tuple_element_t<index, tuple_type>>;
      static_assert(vm_types::is_lua_convertable<arg_type> || aggregate::is_lua_convertable<arg_type>, "call unknown argument");

      if constexpr (aggregate::is_lua_convertable<arg_type>) {
        return self.template operator()<index + 1, result + aggregate::stack_size<arg_type>()>();
      } else {
        return self.template operator()<index + 1, result + 1>();
      }
    }
  }();

  constexpr int param_count = sizeof...(types);
  if constexpr (param_count != 0) {
    (push(state, std::forward<types>(args)), ...);
  }

  if constexpr (result_count == LUA_MULTRET) {
    return operations::pcall(state, stack_slot_count);
  } else {
    return operations::pcall(state, stack_slot_count, result_count);
  }
}
} // namespace maan::stack