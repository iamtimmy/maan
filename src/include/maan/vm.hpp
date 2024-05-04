#pragma once

#include <new>
#include <utility>

#include <lua.hpp>
#include <maan/interface.hpp>
#include <maan/function_type.hpp>
#include <maan/native_function.hpp>

namespace maan {
class MAAN_TRIVIAL_ABI vm {
  lua_State* state;

public:
  void release() {
    state = nullptr;
  }

  [[nodiscard]] bool running() const {
    return state != nullptr;
  }

  [[nodiscard]] size_t working_set() const {
    return operations::working_set(state);
  }

  [[nodiscard]] lua_State* get_state() const {
    return state;
  }

  [[nodiscard]] size_t stack_size() const {
    return operations::size(state);
  }

  void pop(int n = 1) const {
    operations::pop(state, n);
  }

  [[nodiscard]] int execute(std::string_view name, std::string_view code) const {
    return operations::execute(state, name.data(), code.data(), code.size());
  }

  template <typename type>
  [[nodiscard]] decltype(auto) get(int index) const {
    using cvtype = std::remove_cvref_t<type>;

    if constexpr (std::is_same_v<cvtype, function_type>) {
      return function_type(state, index);
    } else {
      return interface::get<type>(state, index);
    }
  }

  template <typename type>
  [[nodiscard]] bool is(int index) const {
    using cvtype = std::remove_cvref_t<type>;

    if constexpr (std::is_same_v<cvtype, function_type>) {
      return operations::is(state, index, vm_type::function);
    } else {
      return interface::is<type>(state, index);
    }
  }

  template <typename type>
  void push(type&& value) const {
    if constexpr (native_function::is_function<type>) {
      return native_function::push(state, std::forward<type>(value));
    } else {
      return interface::push(state, std::forward<type>(value));
    }
  }

  template <int result_count = LUA_MULTRET, typename... types>
  int call(types&&... args) const {
    return interface::call<result_count>(state, std::forward<types>(args)...);
  }

  vm() : state{luaL_newstate()} {
    if (state != nullptr) {
      luaL_openlibs(state);
    }
  }

  explicit vm(lua_State* state) : state{state} {}

  ~vm() {
    if (state == nullptr) {
      return;
    }

    lua_close(state);
  }

  vm(vm&& other) noexcept : state{std::exchange(other.state, nullptr)} {};

  vm& operator=(vm&& other) noexcept {
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