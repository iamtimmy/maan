#include <catch2/catch_all.hpp>

#include <maan.hpp>

TEST_CASE("basic lifetime", "[lifetime]") {
  auto vm = maan::vm();
  REQUIRE(vm.running() == true);
}