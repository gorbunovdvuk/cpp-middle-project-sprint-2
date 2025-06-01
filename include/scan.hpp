#pragma once

#include "parse.hpp"
#include "types.hpp"
#include "format_string.hpp"

namespace stdx {

template<details::format_string format, details::fixed_string string, typename... Ts>
consteval auto scan() {
    static_assert(format.number_placeholders == sizeof...(Ts), "Number placeholders must match");
    return details::parse<format, string, std::tuple<Ts...>>(std::make_index_sequence<format.number_placeholders>{});
}

} // namespace stdx
