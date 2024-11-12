#ifndef __PARSE_STRING_H__
#define __PARSE_STRING_H__

#include <string>
#include <format>
#include <charconv>
#include <optional>
#include <chrono>

#include "enum_bitset.h"

namespace utils {

    template<typename T> struct string_parser;

    template<typename T> requires (std::is_integral_v<T>)
    struct string_parser<T> {
        std::optional<T> operator()(std::string_view str, int base = 10) {
            auto str_begin = str.data();
            auto str_end = str_begin + str.size();
            T result;
            auto [end, ec] = std::from_chars(str_begin, str_end, result, base);
            if (ec == std::errc{} && end == str_end) {
                return result;
            }
            return std::nullopt;
        }
    };

    template<typename T> requires (std::is_floating_point_v<T>)
    struct string_parser<T> {
        std::optional<T> operator()(std::string_view str) {
            auto str_begin = str.data();
            auto str_end = str_begin + str.size();
            T result;
            auto [end, ec] = std::from_chars(str_begin, str_end, result);
            if (ec == std::errc{} && end == str_end) {
                return result;
            }
            return std::nullopt;
        }
    };

    template<> struct string_parser<bool> {
        std::optional<bool> operator()(std::string_view str) {
            if (str == "true") {
                return true;
            } else if (str == "false") {
                return false;
            } else {
                return std::nullopt;
            }
        }
    };

    struct ratio_suffix_pair {
        int num;
        int den;
        std::string_view suffix;

        template<std::integral auto Num, std::integral auto Den>
        constexpr ratio_suffix_pair(std::ratio<Num, Den>, std::string_view suffix)
            : num(Num), den(Den), suffix(suffix) {}
    };

    static constexpr ratio_suffix_pair suffixes[] = {
        {std::chrono::milliseconds::period{}, "ms"},
        {std::chrono::seconds::period{}, "s"},
        {std::chrono::minutes::period{}, "min"},
        {std::chrono::hours::period{}, "h"},
        {std::chrono::days::period{}, "d"},
        {std::chrono::weeks::period{}, "w"},
        {std::chrono::months::period{}, "mon"},
        {std::chrono::years::period{}, "y"}
    };

    template<std::integral auto Num, std::integral auto Den>
    static constexpr std::string_view get_suffix(std::ratio<Num, Den>) {
        auto it = std::ranges::find_if(suffixes, [](const ratio_suffix_pair &pair) {
            return pair.num == Num && pair.den == Den;
        });
        if (it != std::ranges::end(suffixes)) {
            return it->suffix;
        } else {
            throw std::runtime_error("Suffix not found");
        }
    }

    template<typename Rep, typename Period>
    static constexpr std::optional<std::chrono::duration<Rep, Period>> convert_to_duration(double value, std::string_view suffix) {
        auto it = std::ranges::find(suffixes, suffix.substr(suffix.find_first_not_of(" \t")), &ratio_suffix_pair::suffix);
        if (it != std::ranges::end(suffixes)) {
            return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(std::chrono::duration<double>(value * it->num / it->den));
        } else {
            return std::nullopt;
        }
    }

    template<typename Rep, typename Period>
    struct string_parser<std::chrono::duration<Rep, Period>> {
        std::optional<std::chrono::duration<Rep, Period>> operator()(std::string_view str) {
            double value;
            auto [end, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
            if (ec != std::errc{}) {
                return std::nullopt;
            }
            if (end == str.data() + str.size()) {
                return std::chrono::duration<Rep, Period>(static_cast<Rep>(value));
            } else {
                return convert_to_duration<Rep, Period>(value, std::string_view(end, str.data() + str.size()));
            }
        }
    };

    template<enums::enumeral E>
    struct string_parser<enums::bitset<E>> {
        constexpr std::optional<enums::bitset<E>> operator()(std::string_view str) {
            constexpr std::string_view whitespace = " \t";
            enums::bitset<E> result;
            while (true) {
                size_t pos = str.find_first_not_of(whitespace);
                if (pos == std::string_view::npos) break;
                str = str.substr(pos);
                pos = str.find_first_of(whitespace);
                if (auto value = enums::from_string<E>(str.substr(0, pos))) {
                    result.add(*value);
                } else {
                    return std::nullopt;
                }
                if (pos == std::string_view::npos) break;
                str = str.substr(pos);
            }
            return result;
        }
    };

    template<typename T, typename ... Ts>
    std::optional<T> parse_string(std::string_view str, Ts && ... args) {
        return string_parser<T>{}(str, std::forward<Ts>(args) ...);
    }

}

namespace std {

    template<typename Rep, typename Period>
    struct formatter<std::chrono::duration<Rep, Period>> {
        constexpr auto parse(format_parse_context &ctx) {
            return ctx.begin();
        }

        auto format(const std::chrono::duration<Rep, Period> &value, format_context &ctx) const {
            return format_to(ctx.out(), "{} {}", value.count(), utils::get_suffix(typename Period::type{}));
        }
    };

    template<enums::enumeral E>
    struct formatter<E> : formatter<std::string_view> {
        auto format(E value, format_context &ctx) const {
            return formatter<std::string_view>::format(enums::to_string(value), ctx);
        }
    };

    template<enums::enumeral E>
    struct formatter<enums::bitset<E>> : formatter<std::string_view> {
        static constexpr std::string bitset_to_string(enums::bitset<E> value) {
            std::string ret;
            for (E v : enums::enum_values<E>()) {
                if (value.check(v)) {
                    if (!ret.empty()) {
                        ret += ' ';
                    }
                    ret.append(enums::to_string(v));
                }
            }
            return ret;
        }

        auto format(enums::bitset<E> value, format_context &ctx) const {
            return formatter<std::string_view>::format(bitset_to_string(value), ctx);
        }
    };

}

#endif