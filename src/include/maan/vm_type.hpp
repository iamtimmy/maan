#pragma once

namespace maan {
enum class vm_type : int8_t {
  none = -1,
  nil = 0,
  boolean = 1,
  lightuserdata = 2,
  number = 3,
  string = 4,
  table = 5,
  function = 6,
  userdata = 7,
  thread = 8,
};
}