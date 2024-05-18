#pragma once

#include <maan/utilities.hpp>
#include <maan/operatons.hpp>
#include <maan/vm_types.hpp>
#include <maan/aggregate.hpp>
#include <maan/native_function.hpp>

namespace maan::stack {
template <typename type>
[[nodiscard]] MAAN_INLINE bool is(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::is<type>(state, index);
  } else {
    return vm_type::is<type>(state, index);
  }
}

template <typename type>
MAAN_INLINE void push(lua_State* state, type&& value) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::push(state, std::forward<type>(value));
  } else {
    return vm_type::push(state, std::forward<type>(value));
  }
}

template <typename type>
[[nodiscard]] MAAN_INLINE decltype(auto) get(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;

  if constexpr (aggregate::is_lua_convertable<cvtype>) {
    return aggregate::get<type>(state, index);
  } else {
    return vm_type::get<type>(state, index);
  }
}

template <int result_count = LUA_MULTRET, typename... types>
[[nodiscard]] MAAN_INLINE int call(lua_State* state, types&&... args) {
  using info = native_function::info<void(types...)>;

  constexpr int param_count = sizeof...(types);
  if constexpr (param_count != 0) {
    (push(state, std::forward<types>(args)), ...);
  }

  if constexpr (result_count == LUA_MULTRET) {
    return operations::pcall(state, info::requirements.stack_slot_count);
  } else {
    return operations::pcall(state, info::requirements.stack_slot_count, result_count);
  }
}
} // namespace maan::stack