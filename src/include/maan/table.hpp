#pragma once

#include <maan/vm_table.hpp>

#include <maan/stack.hpp>
#include <maan/function.hpp>
#include <maan/native_function.hpp>

namespace maan {
class table {
  vm_table view;
  bool has_ownership;

public:
  MAAN_INLINE table(lua_State* state, int const index) : view{state, operations::abs(state, index)} {
    has_ownership = true;
  }
  MAAN_INLINE table(vm_table const& other) : view{other.state, other.location} {
    has_ownership = true;
  }
  MAAN_INLINE table(vm_table&& other) : view{other.state, other.location} {
    has_ownership = true;
  }

  MAAN_INLINE table(lua_State* state) {
    lua_newtable(state);
    const auto position = operations::abs(state, -1);
    view = {state, position};
    has_ownership = true;
  }

  MAAN_INLINE ~table() {
    if (view.location > 0 && has_ownership) {
      operations::remove(view.state, view.location);
    }
  }

  table(table const&) = delete;
  table& operator=(table const&) = delete;

  MAAN_INLINE table(table&& other) noexcept {
    view = std::exchange(other.view, {});
    has_ownership = other.has_ownership;
  };

  MAAN_INLINE table& operator=(table&& other) noexcept {
    view = std::exchange(other.view, {});
    has_ownership = other.has_ownership;
    return *this;
  }

  MAAN_INLINE void release() {
    has_ownership = false;
  }

  vm_table get_view() const {
    return view;
  }

  template <typename T>
  [[nodiscard]] MAAN_INLINE bool map(auto&& field, auto&& fn) const {
    using field_type = std::remove_cvref_t<decltype(field)>;
    using type = std::remove_cvref_t<T>;

    static constexpr auto field_is_index = std::is_integral_v<field_type>;

    if constexpr (field_is_index) {
      lua_rawgeti(view.state, view.location, std::forward<decltype(field)>(field));
    } else {
      stack::push(view.state, std::forward<decltype(field)>(field));
      lua_rawget(view.state, view.location);
    }

    if constexpr (std::is_same_v<type, function>) {
      if (!operations::is(view.state, -1, vm_type_tag::function)) {
        return false;
      }

      return fn(function(view.state, -1));
    } else {
      if (!stack::is<T>(view.state, -1)) {
        return false;
      }

      return fn(stack::get<T>(view.state, -1));
    }
  }

  template <typename T>
  MAAN_INLINE void set(auto&& field, T&& value) const {
    using type = std::remove_cvref_t<T>;

    using field_type = std::remove_cvref_t<decltype(field)>;
    static constexpr auto field_is_index = std::is_integral_v<field_type>;

    if constexpr (!field_is_index) {
      stack::push(view.state, std::forward<decltype(field)>(field));
    }

    if constexpr (native_function::is_function<T>) {
      native_function::push(view.state, std::forward<T>(value));
    } else if constexpr (native_function::is_cfunction<T>) {
      native_function::push(view.state, std::forward<T>(value));
    } else if constexpr (std::is_same_v<function, type>) {
      operations::copy(view.state, value.stack_location());
    } else {
      stack::push(view.state, std::forward<T>(value));
    }

    if constexpr (field_is_index) {
      lua_rawseti(view.state, view.location, std::forward<decltype(field)>(field));
    } else {
      lua_rawset(view.state, view.location);
    }
  }
};
} // namespace maan