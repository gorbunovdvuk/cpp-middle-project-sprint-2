//
// Created by Dmitry Gorbunov on 30.05.2025.
//

#pragma once
#include <algorithm>

#include "types.hpp"

namespace stdx::details {

template<fixed_string data>
class format_string {
    struct placeholder {
        size_t begin, end;
        char specifier;
    };

    template<fixed_string specifier>
    static constexpr char get_specifier() {
        static_assert(!specifier.empty(), "Empty specifier is provided");
        static_assert(specifier.front() == '{' && specifier.back() == '}', "Specifier must start with '{' and end with '}'");
        if constexpr (specifier == "{}") {
            return '\0';
        } else {
            constexpr auto stripped_specifier = specifier.substr(1, specifier.size() - 2);
            static_assert(
                stripped_specifier.front() == '%' && stripped_specifier.size() == 2,
                "Specifier must start with '%' and contain single character"
            );
            return stripped_specifier.back();
        }
    }

    template<size_t Is>
    static constexpr placeholder internal_get_placeholder_position() {
        constexpr size_t prev_placeholder_end = []() {
            if constexpr (Is == 0) {
                return 0;
            } else {
                return internal_get_placeholder_position<Is - 1>().end;
            }
        }();
        constexpr size_t start_position = data.find("{", prev_placeholder_end);
        static_assert(start_position != data.size(), "Reached end of string before all placeholders were found");
        constexpr size_t end_position = data.find("}", start_position);
        static_assert(end_position != data.size(), "Could not find matching '}' in string");
        return placeholder{
            .begin = start_position,
            .end = end_position + 1,
            .specifier = get_specifier<data.substr(start_position, end_position - start_position + 1)>()
        };
    }

public:
    static constexpr auto string = data;

    static constexpr size_t get_number_placeholders() { return std::count(string.begin(), string.end(), '{'); }

    static constexpr size_t number_placeholders = get_number_placeholders();

    template<size_t... Is>
    static constexpr auto get_placeholders(std::index_sequence<Is...>) {
        return std::array{
            internal_get_placeholder_position<Is>()...
        };
    }

    static constexpr auto placeholder_positions = get_placeholders(std::make_index_sequence<number_placeholders>());

private:
};

namespace format_string_literals {

template<fixed_string string>
consteval auto operator""_fs() {
    return format_string<string>{};
}

}  // namespace format_string_literals

}  // namespace stdx::details
