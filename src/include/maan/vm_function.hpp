#pragma once

#include <maan/stack.hpp>

namespace maan {
class vm_function {
  lua_State* state;
  int absolute_index;

public:
  MAAN_INLINE vm_function(lua_State* state, int index) {
    this->state = state;
    absolute_index = operations::abs(state, index);
  }

  MAAN_INLINE ~vm_function() {
    operations::remove(state, absolute_index);
  }

  vm_function(vm_function const&) = delete;
  vm_function& operator=(vm_function const&) = delete;

  MAAN_INLINE vm_function(vm_function&& other) {
    state = std::exchange(other.state, nullptr);
    absolute_index = std::exchange(other.absolute_index, 0);
  };

  vm_function& operator=(vm_function&&) = delete;

  template <int result_count = LUA_MULTRET, typename... types>
  [[nodiscard]] MAAN_INLINE int call(types&&... args) const {
    operations::copy(state, absolute_index);
    return stack::call<result_count>(state, std::forward<types>(args)...);
  }

  [[nodiscard]] int stack_location() const {
    return absolute_index;
  }
};
} // namespace maan