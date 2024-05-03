#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE("basic functions", "[functions]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);

  REQUIRE(vm.stack_size() == 0);

  vm.push(+[](float a1, float a2) -> float { return a1 + a2; });

  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm.call(100.f, 100.f) == 1);
  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm.is<float>(-1) == true);
  REQUIRE(vm.get<float>(-1) == 200.f);
}
