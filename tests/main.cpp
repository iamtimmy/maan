#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE("basic lifetime", "[lifetime]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);
}

TEST_CASE("advanced lifetime", "[lifetime]") {
  auto vm = maan::vm(nullptr);
  REQUIRE(vm.running() == false);

  auto* state = luaL_newstate();
  luaL_openlibs(state);

  REQUIRE(vm.set_state(state) == nullptr);
  REQUIRE(vm.running() == true);

  auto* extracted_vm = vm.set_state(nullptr);
  REQUIRE(extracted_vm != nullptr);
  REQUIRE(vm.running() == false);

  REQUIRE(vm.set_state(extracted_vm) == nullptr);
  REQUIRE(vm.running() == true);
}