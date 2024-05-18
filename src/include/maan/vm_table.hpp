#pragma once

#include <maan/stack.hpp>
#include <maan/vm_function.hpp>

namespace maan {
class vm_table {
  lua_State* state;
  int absolute_index;

public:
  MAAN_INLINE vm_table(lua_State* state, int index) {
    this->state = state;
    absolute_index = operations::abs(state, index);
  }

  MAAN_INLINE ~vm_table() {
    operations::remove(state, absolute_index);
  }

  vm_table(vm_table const&) = delete;
  vm_table& operator=(vm_table const&) = delete;

  MAAN_INLINE vm_table(vm_table&& other) {
    state = std::exchange(other.state, nullptr);
    absolute_index = std::exchange(other.absolute_index, 0);
  };

  MAAN_INLINE vm_table& operator=(vm_table&& other) {
    state = std::exchange(other.state, nullptr);
    absolute_index = std::exchange(other.absolute_index, 0);
    return *this;
  }

  template <typename type>
  [[nodiscard]] MAAN_INLINE bool map(auto&& field, auto&& fn) const {
    using field_type = std::remove_cvref_t<decltype(field)>;
    using cvtype = std::remove_cvref_t<type>;

    static constexpr auto field_is_index = std::is_integral_v<field_type>;

    if constexpr (field_is_index) {
      lua_rawgeti(state, absolute_index, std::forward<decltype(field)>(field));
    } else {
      stack::push(state, std::forward<decltype(field)>(field));
      lua_rawget(state, absolute_index);
    }

    if constexpr (std::is_same_v<cvtype, vm_function>) {
      if (!operations::is(state, -1, vm_type_tag::function)) {
        return false;
      }

      return fn(vm_function(state, -1));
    } else {
      if (!stack::is<type>(state, -1)) {
        return false;
      }

      return fn(stack::get<type>(state, -1));
    }
  }

  template <typename type>
  MAAN_INLINE void set(auto&& field, type&& value) const {
    using field_type = std::remove_cvref_t<decltype(field)>;
    static constexpr auto field_is_index = std::is_integral_v<field_type>;

    if constexpr (!field_is_index) {
      stack::push(state, std::forward<decltype(field)>(field));
    }

    if constexpr (native_function::is_function<type>) {
      native_function::push(state, std::forward<type>(value));
    } else if constexpr (std::is_same_v<vm_function, type>) {
      operations::copy(state, value.stack_location());
    } else {
      stack::push(state, std::forward<type>(value));
    }

    if constexpr (field_is_index) {
      lua_rawseti(state, absolute_index, std::forward<decltype(field)>(field));
    } else {
      lua_rawset(state, absolute_index);
    }
  }
};
} // namespace maan