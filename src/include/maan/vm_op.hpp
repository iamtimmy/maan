#pragma once

#include <type_traits>

#include <lua.hpp>
#include <maan/utilities.hpp>
#include <maan/vm_type_tag.hpp>

namespace maan::vm_operation {
MAAN_INLINE inline int size(lua_State* state) {
  return lua_gettop(state);
}

template <int index>
MAAN_INLINE constexpr int constant_abs(lua_State* state) {
  if constexpr (index > LUA_REGISTRYINDEX && index < 0) {
    return size(state) + index + 1;
  } else if constexpr (index > 0) {
    return index;
  } else {
    static_assert("bad maan::vm_operation::constant_abs index");
  }
}

MAAN_INLINE inline int abs(lua_State* state, int index) {
  if (index > LUA_REGISTRYINDEX && index < 0) {
    return size(state) + index + 1;
  }

  return index;
}

MAAN_INLINE inline void pop(lua_State* state, int index) {
  lua_settop(state, -index - 1);
}

MAAN_INLINE inline void clear(lua_State* state) {
  lua_settop(state, 0);
}

MAAN_INLINE inline void remove(lua_State* state, int index) {
  lua_remove(state, index);
}

MAAN_INLINE inline void insert(lua_State* state, int index) {
  lua_insert(state, index);
}

MAAN_INLINE inline vm_type_tag type(lua_State* state, int index) {
  return static_cast<vm_type_tag>(lua_type(state, index));
}

MAAN_INLINE inline bool is(lua_State* state, int index, vm_type_tag type) {
  return lua_type(state, index) == utilities::to_underlying(type);
}

MAAN_INLINE inline size_t working_set(lua_State* state) {
  return lua_gc(state, LUA_GCCOUNT, 0);
}

MAAN_INLINE inline void stop_gc(lua_State* state) {
  lua_gc(state, LUA_GCSTOP, 0);
}

MAAN_INLINE inline void start_gc(lua_State* state) {
  lua_gc(state, LUA_GCRESTART, 0);
}

MAAN_INLINE inline void perform_gc_cycle(lua_State* state) {
  lua_gc(state, LUA_GCCOLLECT, 0);
}

MAAN_INLINE inline void perform_gc_step(lua_State* state) {
  static constexpr auto step_ratio = 150;
  lua_gc(state, LUA_GCSTEP, step_ratio);
}

MAAN_INLINE inline int error_handler(lua_State* state) {
  luaL_traceback(state, state, lua_tolstring(state, -1, nullptr), 0);
  return 1;
}

MAAN_INLINE inline int pcall(lua_State* state, int nargs, int result_count) {
  // expected stack layout:
  // - params
  // - chunk

  // determine position of the error handler function
  const auto error_function_pos = size(state) - nargs;
  lua_pushcclosure(state, error_handler, 0);

  // move error handler to the top of the stack
  insert(state, error_function_pos);

  // expected stack layout:
  // - params
  // - chunk
  // - error handler

  if (const auto result = lua_pcall(state, nargs, result_count, error_function_pos); result == 0) [[likely]]
  {
    remove(state, error_function_pos);
    return result_count == LUA_MULTRET ? size(state) : result_count;
  } else {
    switch (result) {
    case LUA_ERRRUN: {
      remove(state, error_function_pos);
      return -1;
    }
    case LUA_ERRMEM: {
      clear(state);
      return -2;
    }
    case LUA_ERRERR: {
      clear(state);
      return -3;
    }
    default: {
      utilities::assume_unreachable();
    }
    }
  }
}

MAAN_INLINE inline int pcall(lua_State* state, int nargs) {
  const auto stack_size = size(state);

  // expected stack layout:
  // - params
  // - chunk

  // determine position of the error handler function
  const auto error_function_pos = size(state) - nargs;
  lua_pushcclosure(state, error_handler, 0);

  // move error handler to the top of the stack
  insert(state, error_function_pos);

  // expected stack layout:
  // - params
  // - chunk
  // - error handler

  if (const auto result = lua_pcall(state, nargs, LUA_MULTRET, error_function_pos); result == 0) [[likely]]
  {
    remove(state, error_function_pos);
    return size(state) - (stack_size - 1 - nargs);
  } else {
    switch (result) {
    case LUA_ERRRUN: {
      remove(state, error_function_pos);
      return -1;
    }
    case LUA_ERRMEM: {
      clear(state);
      return -2;
    }
    case LUA_ERRERR: {
      clear(state);
      return -3;
    }
    default: {
      utilities::assume_unreachable();
    }
    }
  }
}

MAAN_INLINE inline int pcall(lua_State* state) {
  return pcall(state, size(state) - 1);
}

MAAN_INLINE inline int execute(lua_State* state, const char* name, const char* code, size_t size) {
  if (const auto result = luaL_loadbuffer(state, code, size, name); result == LUA_OK)
  {
    return pcall(state, 0);
  } else {
    if (result == LUA_ERRSYNTAX) {
      return -1;
    }

    clear(state);
    return -1;
  }
}
} // namespace maan::vm_operation