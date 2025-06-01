#pragma once
#include <filesystem>

namespace stdx::details {

template<auto...>
consteval void debug() {
    static_assert(false);
}

template<template<auto...> typename V, typename T>
struct is_instance_of : std::false_type {};

template<template<auto...> typename V, auto... Args>
struct is_instance_of<V, V<Args...>> : std::true_type {};

template<template<auto...> typename V, typename T>
static constexpr bool is_instance_of_v = is_instance_of<V, T>::value;

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
class scan_result {
public:
    consteval scan_result(std::tuple<Ts...>&& tuple): values_(std::move(tuple)) {}

    consteval const auto& values() const { return values_; }

private:
    std::tuple<Ts...> values_;
};

template<size_t N = 256>
struct fixed_string {
    std::array<char, N> buffer{};

    fixed_string() = default;

    template<size_t M>
    requires (M <= N)
    constexpr fixed_string(const char (&array)[M]): fixed_string() {
        std::ranges::copy(std::span(array), std::begin(buffer));
    }

    constexpr fixed_string(const char* begin, const char* end): fixed_string() {
        std::copy(begin, end, buffer.data());
    }

    constexpr fixed_string substr(size_t start, size_t size) const {
        return fixed_string(buffer.data() + start, buffer.data() + start + size);
    }

    constexpr auto begin() const { return buffer.data(); }

    constexpr auto end() const { return buffer.data() + size(); }

    constexpr const char* data() const { return buffer.data(); }

    constexpr size_t size() const {
        return std::find(buffer.begin(), buffer.end(), '\0') - buffer.begin();
    }

    constexpr bool empty() const { return size() == 0; }

    constexpr char operator[](size_t index) const { return buffer[index]; }

    constexpr char front() const { return buffer[0]; }

    constexpr char back() const { return buffer[size() - 1]; }

    constexpr std::string_view view() const { return std::string_view(buffer.data(), size()); }

    constexpr size_t find(std::string_view s, size_t pos = 0) const {
        if (s.empty()) {
            return size();
        }
        while (pos + s.size() <= size() && substr(pos, s.size()) != s) {
            pos++;
        }
        return pos;
    }

    friend constexpr bool operator==(const fixed_string& s, const std::string_view& sv) {
        return s.view() == sv;
    }

    template<size_t M>
    constexpr bool operator==(const fixed_string<M>& t) const {
        return view() == t.view();
    }
};

template<size_t N>
fixed_string(const char (&array)[N]) -> fixed_string<N>;

template<typename T>
concept SignedInt = std::is_same_v<std::remove_cv_t<T>, int8_t> ||
                    std::is_same_v<std::remove_cv_t<T>, int16_t> ||
                    std::is_same_v<std::remove_cv_t<T>, int32_t> ||
                    std::is_same_v<std::remove_cv_t<T>, int64_t>;

template<typename T>
concept UnsignedInt = std::is_same_v<std::remove_cv_t<T>, uint8_t> ||
                      std::is_same_v<std::remove_cv_t<T>, uint16_t> ||
                      std::is_same_v<std::remove_cv_t<T>, uint32_t> ||
                      std::is_same_v<std::remove_cv_t<T>, uint64_t>;

template<typename T>
concept AnyInt = SignedInt<T> || UnsignedInt<T>;

template<typename T>
concept StringType = std::is_same_v<std::remove_cv_t<T>, std::string_view> ||
                     is_instance_of_v<fixed_string, std::remove_cv_t<T>>;

template<typename T>
concept AllowedTypes = SignedInt<T> || UnsignedInt<T> || StringType<T>;

} // namespace stdx::details
