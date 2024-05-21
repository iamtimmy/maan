#pragma once

#include <lua.hpp>

namespace maan {
struct vm_function {
  lua_State* state;
  int location;
};
} // namespace maan