#pragma once

#include <maan/vm_types.hpp>
#include <maan/utilities.hpp>
#include <tuple>

namespace maan::aggregate {
template <typename T>
concept is_lua_convertable = requires(T) { requires std::is_class_v<std::remove_cvref_t<T>> && utilities::member_countable<std::remove_cvref_t<T>>; };

template <is_lua_convertable type>
static constexpr int stack_size() {
  using cvtype = std::remove_cvref_t<type>;
  return utilities::member_count<cvtype>();
}

template <size_t index = 0, typename... types>
static void set_tuple(lua_State* state, int stack_index, std::tuple<types...>& tuple) {
  if constexpr (index == sizeof...(types)) {
    return;
  } else {
    using argument_type = std::tuple_element_t<index, std::tuple<types...>>;

    std::get<index>(tuple) = vm_type::get<argument_type>(state, stack_index + index);
    return set_tuple<index + 1>(state, stack_index, tuple);
  }
}

template <size_t index = 0, typename... types>
static bool check(lua_State* state, int stack_index) {
  if constexpr (index == sizeof...(types)) {
    return true;
  } else {
    using argument_type = std::tuple_element_t<index, std::tuple<types...>>;

    if (!vm_type::is<argument_type>(state, stack_index + index)) [[unlikely]] {
      return false;
    } else {
      return check<index + 1, types...>(state, stack_index);
    }
  }
}

template <is_lua_convertable type>
static bool is(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;

  static constexpr auto count = utilities::member_count<cvtype>();

  const auto stack_size = vm_operation::size(state);
  const auto start_index = vm_operation::abs(state, index);
  const auto stop_index = start_index + count - 1;

  if (stop_index > stack_size) [[unlikely]] {
    return false;
  }

  const auto fn = [state, start_index]<typename... types>() { return check<0, types...>(state, start_index); };

  return utilities::visit_members_types<type>(type{}, fn);
}

static void push(lua_State* state, is_lua_convertable auto&& value) {
  utilities::visit_members(std::forward<decltype(value)>(value), [state](auto&&... members) { (vm_type::push(state, members), ...); });
}

template <is_lua_convertable type>
static decltype(auto) get(lua_State* state, int index) {
  using cvtype = std::remove_cvref_t<type>;
  static constexpr auto count = stack_size<cvtype>();

  const auto stack_start_index = vm_operation::abs(state, index);

  const auto fn = [state, stack_start_index]<typename... types>() {
    std::tuple<types...> member_values;
    set_tuple(state, stack_start_index, member_values);

    const auto fn = [](auto&&... params) -> cvtype { return cvtype{params...}; };

    return std::apply(fn, member_values);
  };

  return utilities::visit_members_types<type>(type{}, fn);
}
} // namespace maan::aggregate