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

TEST_CASE("stack functions", "[functions]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);

  REQUIRE(vm.stack_size() == 0);

  static constexpr std::string_view code = "return function(a) return a + 10 end";

  REQUIRE(vm.execute("code", code) == 1);
  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm.is<maan::function_type>(-1));

  const auto fn = vm.get<maan::function_type>(-1);
  REQUIRE(fn.call(100) == 1);

  REQUIRE(vm.is<int>(-1));
  REQUIRE(vm.get<int>(-1) == 110);

  vm.pop();
  REQUIRE(vm.stack_size() == 0);
}