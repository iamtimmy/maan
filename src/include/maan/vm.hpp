#pragma once

#include <new>
#include <utility>

#include <lua.hpp>
#include <maan/stack.hpp>
#include <maan/vm_function.hpp>
#include <maan/vm_table.hpp>
#include <maan/native_function.hpp>

namespace maan {
class vm {
  lua_State* state;

public:
  [[nodiscard]] MAAN_INLINE bool running() const {
    return state != nullptr;
  }

  [[nodiscard]] MAAN_INLINE size_t working_set() const {
    return operations::working_set(state);
  }

  [[nodiscard]] MAAN_INLINE lua_State* get_state() const {
    return state;
  }

  [[nodiscard]] MAAN_INLINE size_t stack_size() const {
    return operations::size(state);
  }

  MAAN_INLINE void pop(int n = 1) const {
    operations::pop(state, n);
  }

  [[nodiscard]] MAAN_INLINE int execute(std::string_view name, std::string_view code) const {
    return operations::execute(state, name.data(), code.data(), code.size());
  }

  template <typename type>
  [[nodiscard]] MAAN_INLINE decltype(auto) get(int index) const {
    using cvtype = std::remove_cvref_t<type>;

    if constexpr (std::is_same_v<cvtype, vm_function>) {
      return vm_function(state, index);
    } else {
      return stack::get<type>(state, index);
    }
  }

  template <typename type>
  [[nodiscard]] MAAN_INLINE bool is(int index) const {
    using cvtype = std::remove_cvref_t<type>;

    if constexpr (std::is_same_v<cvtype, vm_function>) {
      return operations::is(state, index, vm_type_tag::function);
    } else {
      return stack::is<type>(state, index);
    }
  }

  template <typename type>
  MAAN_INLINE void push(type&& value) const {
    if constexpr (native_function::is_function<type>) {
      return native_function::push(state, std::forward<type>(value));
    } else {
      return stack::push(state, std::forward<type>(value));
    }
  }

  template <int result_count = LUA_MULTRET, typename... types>
  [[nodiscard]] MAAN_INLINE int call(types&&... args) const {
    return stack::call<result_count>(state, std::forward<types>(args)...);
  }

  [[nodiscard]] MAAN_INLINE vm_table get_globals() const {
    lua_pushvalue(state, LUA_GLOBALSINDEX);
    return vm_table(state, -1);
  }

  MAAN_INLINE lua_State* set_state(lua_State* new_state) {
    return std::exchange(state, new_state);
  }

  MAAN_INLINE vm() : state{luaL_newstate()} {
    if (state != nullptr) {
      luaL_openlibs(state);
    }
  }

  MAAN_INLINE explicit vm(lua_State* state) : state{state} {}

  MAAN_INLINE ~vm() {
    if (state == nullptr) {
      return;
    }

    lua_close(state);
  }

  MAAN_INLINE vm(vm&& other) noexcept : state{std::exchange(other.state, nullptr)} {};

  MAAN_INLINE vm& operator=(vm&& other) noexcept {
    if (this != &other) {
      state = std::exchange(other.state, nullptr);
    }

    return *this;
  }

  // delete the copy contructor and copy assignment operator
  // there is no good way to duplicate lua's state
  vm(vm const&) = delete;
  vm& operator=(vm const&) = delete;
};
} // namespace maan