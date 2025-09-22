#pragma once

#include <maan/vm_types.hpp>
#include <maan/utilities.hpp>

#include <maan/vm_function.hpp>
#include <maan/vm_table.hpp>

namespace maan::aggregate {
template <typename T>
concept is_lua_convertable = std::is_class_v<std::remove_cvref_t<T>> && utilities::member_countable<std::remove_cvref_t<T>> &&
                             !std::is_same_v<vm_function, std::remove_cvref_t<T>> && !std::is_same_v<vm_table, std::remove_cvref_t<T>>;

template <is_lua_convertable T>
MAAN_INLINE static constexpr int stack_size() {
  using type = std::remove_cvref_t<T>;
  return utilities::member_count<type>();
}

template <size_t index = 0, typename... types>
MAAN_INLINE static void set_tuple(lua_State* state, int stack_index, std::tuple<types...>& tuple) {
  if constexpr (index == sizeof...(types)) {
    return;
  } else {
    using argument_type = std::tuple_element_t<index, std::tuple<types...>>;

    std::get<index>(tuple) = vm_types::get<argument_type>(state, stack_index + index);
    return set_tuple<index + 1>(state, stack_index, tuple);
  }
}

template <size_t index = 0, typename... Ts>
MAAN_INLINE static bool check(lua_State* state, int const stack_index) {
  if constexpr (index == sizeof...(Ts)) {
    return true;
  } else {
    using argument_type = std::tuple_element_t<index, std::tuple<Ts...>>;

    if (!vm_types::is<argument_type>(state, stack_index + index)) [[unlikely]] {
      return false;
    } else {
      return check<index + 1, Ts...>(state, stack_index);
    }
  }
}

template <is_lua_convertable T>
MAAN_INLINE static bool is(lua_State* state, int const index) {
  using type = std::remove_cvref_t<T>;

  static constexpr auto count = utilities::member_count<type>();

  const auto stack_size = operations::size(state);
  const auto start_index = operations::abs(state, index);
  const auto stop_index = start_index + count - 1;

  if (stop_index > stack_size) [[unlikely]] {
    return false;
  }

  const auto fn = [state, start_index]<typename... types>() { return check<0, types...>(state, start_index); };

  return utilities::visit_members_types<T>(T{}, fn);
}

MAAN_INLINE static void push(lua_State* state, is_lua_convertable auto&& value) {
  utilities::visit_members(std::forward<decltype(value)>(value), [state](auto&&... members) { (vm_types::push(state, members), ...); });
}

template <is_lua_convertable T>
MAAN_INLINE static decltype(auto) get(lua_State* state, int const index) {
  using type = std::remove_cvref_t<T>;
  static constexpr auto count = stack_size<type>();

  const auto stack_start_index = operations::abs(state, index);

  const auto fn = [state, stack_start_index]<typename... types>() {
    std::tuple<types...> member_values;
    set_tuple(state, stack_start_index, member_values);

    const auto aggregate_constructor = [](auto&&... params) -> type { return type{params...}; };
    return std::apply(aggregate_constructor, member_values);
  };

  return utilities::visit_members_types<T>(T{}, fn);
}

template <is_lua_convertable T>
MAAN_INLINE std::string_view name([[maybe_unused]] lua_State* state, [[maybe_unused]] int index) {
  using type = std::remove_cvref_t<T>;
  return utilities::type_tag<type>::to_string();
}
} // namespace maan::aggregate