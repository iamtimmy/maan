#pragma once

#include <maan/interface.hpp>

namespace maan {
class function_type {
  int absolute_index;
  lua_State* state;

public:
  MAAN_INLINE function_type(lua_State* state, int index) : state{state}, absolute_index{operations::abs(state, index)} {}

  template <int result_count = LUA_MULTRET, typename... types>
  [[nodiscard]] MAAN_INLINE int call(types&&... args) const {
    return interface::call<result_count>(state, std::forward<types>(args)...);
  }
};
} // namespace maan