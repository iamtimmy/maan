#pragma once

#include <lua.hpp>

namespace maan {
struct vm_table {
  lua_State* state;
  int location;
};
} // namespace maan