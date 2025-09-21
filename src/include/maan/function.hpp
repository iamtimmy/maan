#pragma once

#include <maan/vm_function.hpp>
#include <maan/stack.hpp>

namespace maan {
class function {
  vm_function view;

public:
  MAAN_INLINE function(lua_State* state, int const index) : view(state, operations::abs(state, index)) {}
  MAAN_INLINE function(vm_function const& other) : view{.state = other.state, .location = other.location} {}
  MAAN_INLINE function(vm_function&& other) : view{.state = other.state, .location = other.location} {}

  MAAN_INLINE ~function() {
    operations::remove(view.state, view.location);
  }

  function(function const&) = delete;
  function& operator=(function const&) = delete;

  MAAN_INLINE function(function&& other) noexcept {
    view = std::exchange(other.view, {});
  };

  MAAN_INLINE function& operator=(function&& other) noexcept {
    view = std::exchange(other.view, {});
    return *this;
  }

  template <int result_count = LUA_MULTRET, typename... types>
  [[nodiscard]] MAAN_INLINE int call(types&&... args) const {
    operations::copy(view.state, view.location);
    return stack::call<result_count>(view.state, std::forward<types>(args)...);
  }

  [[nodiscard]] MAAN_INLINE int get_location() const {
    return view.location;
  }
};
} // namespace maan