#include "scan.hpp"

using namespace stdx::details::format_string_literals;
using stdx::details::debug;
using stdx::details::fixed_string;
using stdx::details::format_string;

template<format_string format, fixed_string string, typename... Expected>
consteval auto run_and_check_types() {
    constexpr auto result =
        stdx::scan<format, string, Expected...>();
    static_assert(
        std::is_same_v<decltype(result.values()), const std::tuple<Expected...>&>,
        "Output types do not match input types"
    );
    return result;
}

template<format_string format, fixed_string string, auto... Expected>
consteval auto run_full_check() {
    constexpr auto result = run_and_check_types<format, string, decltype(Expected)...>();
    constexpr auto expected = std::make_tuple(Expected...);
    static_assert(result.values() == expected, "Result is incorrect");
    return result;
}

constinit auto simple_test = run_full_check<
    "I want to sum {} and {%d} numbers."_fs,
    "I want to sum 3 and 42 numbers.",
    static_cast<int32_t>(3),
    static_cast<int64_t>(42)
>();

constinit auto test_specifiers = run_full_check<
    "name={%s},id={%u},number={%d}"_fs,
    "name=aba,id=123,number=-123",
    fixed_string<>("aba"),
    static_cast<uint32_t>(123),
    static_cast<int32_t>(-123)
>();

constinit auto test_all_types = run_and_check_types<
    "{%u} {%u} {%u} {%u} {%d} {%d} {%d} {%d} {%s} {%s}"_fs,
    "0 0 0 0 0 0 0 0 0 0",
    uint8_t, uint16_t, uint32_t, uint64_t,
    int8_t, int16_t, int32_t, int64_t,
    fixed_string<>, std::string_view
>();

constinit auto test_const_types = run_and_check_types<
    "{%u} {%u} {%u} {%u} {%d} {%d} {%d} {%d} {%s} {%s}"_fs,
    "0 0 0 0 0 0 0 0 0 0",
    const uint8_t, const uint16_t, const uint32_t, const uint64_t,
    const int8_t, const int16_t, const int32_t, const int64_t,
    const fixed_string<>, const std::string_view
>();

constinit auto test_minus_integers = run_full_check<
    "{%d}"_fs,
    "-2",
    static_cast<int32_t>(-2)
>();

constinit auto test_squashed_number = run_full_check<
    "g{%d}e"_fs,
    "g42e",
    static_cast<int32_t>(42)
>();

constinit auto test_empty_string = run_full_check<
    "g{}e"_fs,
    "ge",
    fixed_string<>("")
>();

constinit auto test_empty_string_in_empty_context = run_full_check<
    "{}"_fs,
    "",
    fixed_string<>("")
>();

constinit auto test_whole_string = run_full_check<
    "{}"_fs,
    "Hello world!",
    fixed_string<>("Hello world!")
>();

constinit auto test_overflow = run_full_check<
    "{%d}"_fs,
    "-128",
    static_cast<int8_t>(-128)
>();