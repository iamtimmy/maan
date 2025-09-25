#pragma once

#include <maan/vm_function.hpp>
#include <maan/stack.hpp>

namespace maan {
class function {
  vm_function view;
  bool has_ownership;

public:
  MAAN_INLINE function(lua_State* state, int const index) : view(state, operations::abs(state, index)) {
    has_ownership = true;
  }
  MAAN_INLINE function(vm_function const& other) : view{.state = other.state, .location = other.location} {
    has_ownership = true;
  }
  MAAN_INLINE function(vm_function&& other) : view{.state = other.state, .location = other.location} {
    has_ownership = true;
  }

  MAAN_INLINE ~function() {
    if (view.location > 0 && has_ownership) {
      operations::remove(view.state, view.location);
    }
  }

  function(function const&) = delete;
  function& operator=(function const&) = delete;

  MAAN_INLINE function(function&& other) noexcept {
    view = std::exchange(other.view, {});
    has_ownership = other.has_ownership;
  };

  MAAN_INLINE function& operator=(function&& other) noexcept {
    view = std::exchange(other.view, {});
    has_ownership = other.has_ownership;
    return *this;
  }

  MAAN_INLINE void release() {
    has_ownership = false;
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