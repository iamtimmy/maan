#pragma once

#include <maan/vm.hpp>

namespace maan {
  class MAAN_TRIVIAL_ABI vm_view {
    vm& state;
  public:
    vm_view(vm& vm) : state{vm} {}

    operator vm&() { return state; }
    operator vm const&() const { return state; }

    vm* operator->() { return &state; }
    vm const* operator->() const { return &state; }
  };
}