#pragma once

#include <expected>
#include <utility>

#include "format_string.hpp"
#include "types.hpp"

namespace stdx::details {

template<typename ElementType, char c>
consteval void check_specifier() {
    static_assert(
        c == '\0' ||
        (UnsignedInt<ElementType> && (c == 'u')) ||
        (SignedInt<ElementType> && (c == 'd')) ||
        (StringType<ElementType> && (c == 's')),
        "Specifier does not match element type"
    );
}

template<StringType ElementType, fixed_string view>
consteval ElementType parse_value() {
    return ElementType(view.begin(), view.end());
}

template<typename ElementType, fixed_string digits>
consteval auto parse_without_sign() {
    static_assert(!digits.empty(), "Digits must be non-empty");
    static_assert(
        std::ranges::all_of(digits, [](const char c) { return '0' <= c && c <= '9'; }),
        "Digits are expected, but unknown symbol is found"
    );

    std::remove_cv_t<ElementType> value = 0;
    for (const auto c : digits) {
        value = value * 10 + (c - '0');
    }
    return value;
};

template<AnyInt ElementType, fixed_string string>
consteval ElementType parse_value() {
    if constexpr ((!SignedInt<ElementType> && string.front() != '+') ||
        (SignedInt<ElementType> && string.front() != '+' && string.front() != '-')) {
        return parse_without_sign<ElementType, string>();
    } else {
        constexpr bool negative = string.front() == '-';
        constexpr auto value = parse_without_sign<ElementType, string.substr(1, string.size() - 1)>();
        return negative ? -value : value;
    }
}

struct placeholder_source {
    size_t begin, end;
    char specifier;
};

template<size_t Is, format_string format, fixed_string string>
consteval placeholder_source get_current_source_for_parsing() {
    constexpr size_t prev_format_end = []() {
        if constexpr (Is == 0) {
            return 0;
        } else {
            return format.placeholder_positions[Is - 1].end;
        }
    }();

    constexpr size_t prev_string_end = []() {
        if constexpr (Is == 0) {
            return 0;
        } else {
            return get_current_source_for_parsing<Is - 1, format, string>().end;
        }
    }();

    constexpr size_t prefix_size = format.placeholder_positions[Is].begin - prev_format_end;
    constexpr auto format_prefix = format.string.substr(prev_format_end, prefix_size);
    constexpr auto string_prefix = string.substr(prev_string_end, prefix_size);
    static_assert(format_prefix == string_prefix, "Prefixes do not match in format and string");

    constexpr size_t next_format_start = []() {
        if constexpr (Is + 1 == format.number_placeholders) {
            return format.string.size();
        } else {
            return format.placeholder_positions[Is + 1].begin;
        }
    }();
    constexpr auto suffix_size = next_format_start - format.placeholder_positions[Is].end;
    constexpr auto format_suffix = format.string.substr(format.placeholder_positions[Is].end, suffix_size);

    constexpr size_t string_suffix_start_position = prev_string_end + prefix_size;
    constexpr size_t string_suffix_end_position = string.find(format_suffix.view(), string_suffix_start_position);
    static_assert(
        string_suffix_end_position + format_suffix.size() <= string.size(),
        "Cound not find matching suffix in string"
    );

    return placeholder_source{
        .begin = string_suffix_start_position,
        .end = string_suffix_end_position,
        .specifier = format.placeholder_positions[Is].specifier,
    };
}

template<size_t Is, format_string format, fixed_string string, typename ElementType>
consteval ElementType parse_input() {
    static_assert(AllowedTypes<ElementType>, "Provided type is not supported");

    constexpr auto source = get_current_source_for_parsing<Is, format, string>();
    check_specifier<ElementType, source.specifier>();

    constexpr auto current_string = string.substr(source.begin, source.end - source.begin);
    return parse_value<ElementType, current_string>();
}

template<format_string format, fixed_string string, typename ArgumentsTuple, size_t... Is>
consteval auto parse(std::index_sequence<Is...>) {
    return scan_result{ArgumentsTuple(
        parse_input<Is, format, string, std::tuple_element_t<Is, ArgumentsTuple>>()...
    )};
}

} // namespace stdx::details
