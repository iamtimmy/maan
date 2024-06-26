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

  REQUIRE(vm.is<maan::function>(-1) == true);

  {
    const auto fn = vm.get<maan::function>(-1);
    REQUIRE(fn.call(100) == 1);
    REQUIRE(vm.stack_size() == 2);

    REQUIRE(vm.is<int>(-1));
    REQUIRE(vm.get<int>(-1) == 110);

    vm.pop();
    REQUIRE(vm.stack_size() == 1);

    REQUIRE(fn.call(200) == 1);
    REQUIRE(vm.stack_size() == 2);

    REQUIRE(vm.is<int>(-1));
    REQUIRE(vm.get<int>(-1) == 210);

    vm.pop();
    REQUIRE(vm.stack_size() == 1);
  }

  REQUIRE(vm.stack_size() == 0);
}

TEST_CASE("nested stack functions", "[functions]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);

  REQUIRE(vm.stack_size() == 0);

  static constexpr std::string_view code = "function plus_ten(v) return v + 10 end; return function(fn, v) return fn(plus_ten, v) end";

  REQUIRE(vm.execute("code", code) == 1);
  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm.is<maan::function>(-1) == true);

  {
    const auto lambda = +[](maan::vm_function const& fn, int v) -> int {
      const auto function = maan::function(fn);

      REQUIRE(function.call(v) == 1);

      REQUIRE(maan::vm_types::is<int>(fn.state, -1));

      const auto result = maan::vm_types::get<int>(fn.state, -1);
      REQUIRE(result == 110);
      return result;
    };

    const auto fn = vm.get<maan::function>(-1);
    REQUIRE(fn.call(lambda, 100) == 1);
    REQUIRE(vm.stack_size() == 2);

    REQUIRE(vm.is<int>(-1));
    REQUIRE(vm.get<int>(-1) == 110);

    vm.pop();
    REQUIRE(vm.stack_size() == 1);
  }

  REQUIRE(vm.stack_size() == 0);
}