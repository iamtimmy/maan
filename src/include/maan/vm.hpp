#pragma once

#include <maan/stack.hpp>
#include <maan/function.hpp>
#include <maan/table.hpp>
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

  MAAN_INLINE void pop(int const n = 1) const {
    operations::pop(state, n);
  }

  [[nodiscard]] MAAN_INLINE int load(const char* name, std::string_view const code) const {
    return operations::load(state, name, code.data(), code.size());
  }

  [[nodiscard]] MAAN_INLINE int load(const char* name, std::string_view const code, int const env_table_index) const {
    return operations::load(state, name, code.data(), code.size(), env_table_index);
  }

  [[nodiscard]] MAAN_INLINE int load(const char* name, const char* code, size_t const size) const {
    return operations::load(state, name, code, size);
  }

  [[nodiscard]] MAAN_INLINE int load(const char* name, const char* code, size_t const size, int const env_table_index) const {
    return operations::load(state, name, code, size, env_table_index);
  }

  [[nodiscard]] MAAN_INLINE int execute(const char* name, const char* code, size_t const size) const {
    return operations::execute(state, name, code, size);
  }

  [[nodiscard]] MAAN_INLINE int execute(const char* name, const char* code, size_t const size, int const env_table_index) const {
    return operations::execute(state, name, code, size, env_table_index);
  }

  [[nodiscard]] MAAN_INLINE int execute(const char* name, std::string_view const code) const {
    return operations::execute(state, name, code.data(), code.size());
  }

  [[nodiscard]] MAAN_INLINE int execute(const char* name, std::string_view const code, int const env_table_index) const {
    return operations::execute(state, name, code.data(), code.size(), env_table_index);
  }

  template <typename T>
  [[nodiscard]] MAAN_INLINE decltype(auto) get(int const index) const {
    using type = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<type, function>) {
      return function(state, index);
    } else {
      return stack::get<T>(state, index);
    }
  }

  template <typename T>
  [[nodiscard]] MAAN_INLINE bool is(int const index) const {
    using type = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<type, function>) {
      return operations::is(state, index, vm_type_tag::function);
    } else {
      return stack::is<T>(state, index);
    }
  }

  MAAN_INLINE void push_cfunction(lua_CFunction const fn, int const count = 0) const {
    native_function::push(state, fn, count);
  }

  template <typename T>
  MAAN_INLINE void push(T&& value) const {
    if constexpr (native_function::is_function<T>) {
      return native_function::push(state, std::forward<T>(value));
    } else if constexpr (native_function::is_cfunction<T>) {
      return native_function::push(state, std::forward<T>(value));
    } else {
      return stack::push(state, std::forward<T>(value));
    }
  }

  template <int result_count = LUA_MULTRET, typename... Ts>
  [[nodiscard]] MAAN_INLINE int call(Ts&&... args) const {
    constexpr auto stack_slot_count = []<size_t index = 0, size_t result = 0>(this auto&& self) {
      using tuple_type = std::tuple<Ts...>;

      if constexpr (index == sizeof...(Ts)) {
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

    return stack::call<result_count>(state, std::forward<Ts>(args)...);
  }

  [[nodiscard]] MAAN_INLINE table get_globals() const {
    lua_pushvalue(state, LUA_GLOBALSINDEX);
    return {state, -1};
  }

  MAAN_INLINE lua_State* set_state(lua_State* new_state) {
    return std::exchange(state, new_state);
  }

  MAAN_INLINE void release() {
    state = nullptr;
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