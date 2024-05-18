#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE("basic tables", "[tables]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);

  {
    const auto table = vm.get_globals();
    table.set("test", 100);

    REQUIRE(vm.stack_size() == 1);

    const auto lambda = [](int test) {
      REQUIRE(test == 100);
      return true;
    };

    REQUIRE(table.map<int>("test", lambda));

    vm.pop();

    REQUIRE(vm.stack_size() == 1);
  }

  REQUIRE(vm.stack_size() == 0);
}