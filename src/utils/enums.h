#ifndef __ENUMS_H__
#define __ENUMS_H__

#include <reflect>

#include "json_serial.h"
#include "static_map.h"
#include "visit_indexed.h"

namespace enums {

    template<typename T> concept enumeral = std::is_enum_v<T>;

    template<enumeral auto E> struct enum_tag_t {};

    template<enumeral auto E> inline constexpr enum_tag_t<E> enum_tag;

    template<enumeral T, typename ISeq> struct build_enum_values;
    template<enumeral T, size_t ... Is> struct build_enum_values<T, std::index_sequence<Is ...>> {
        static constexpr std::array value { static_cast<T>(reflect::enumerators<T>[Is].first) ... };
    };

    template<enumeral T> constexpr const auto &enum_values() {
        return build_enum_values<T, std::make_index_sequence<reflect::enumerators<T>.size()>>::value;
    }

    template<enumeral T>
    constexpr bool is_linear_enum() {
        size_t i=0;
        for (T value : enum_values<T>()) {
            if (static_cast<size_t>(value) != i) {
                return false;
            }
            ++i;
        }
        return true;
    }

    template<enumeral T>
    constexpr size_t indexof(T value) {
        constexpr const auto &values = enum_values<T>();
        if constexpr (is_linear_enum<T>()) {
            size_t result = static_cast<size_t>(value);
            if (result >= 0 && result <= static_cast<size_t>(values.back())) {
                return result;
            }
        } else {
            for (size_t i=0; i<values.size(); ++i) {
                if (values[i] == value) {
                    return i;
                }
            }
        }
        throw std::out_of_range("invalid enum index");
    }

    template<enumeral T>
    constexpr bool is_between(T value, T min, T max) {
        size_t index = indexof(value);
        return index >= indexof(min) && index <= indexof(max);
    }

    template<enumeral T>
    constexpr std::string_view to_string(T input) {
        return reflect::enumerators<T>[indexof(input)].second;
    }

    template<enumeral T>
    static constexpr auto names_to_values_map = []<size_t ... Is>(std::index_sequence<Is ...>) {
        constexpr auto &values = enum_values<T>();
        return utils::make_static_map<std::string_view, T>({
            { to_string(values[Is]), values[Is] } ...
        });
    }(std::make_index_sequence<enum_values<T>().size()>());

    template<enumeral T>
    constexpr std::optional<T> from_string(std::string_view str) {
        const auto &names_map = names_to_values_map<T>;
        if (auto it = names_map.find(str); it != names_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    template<typename RetType, typename Visitor, enumeral ... E>
    RetType visit_enum(Visitor &&visitor, E ... values) {
        return utils::visit_indexed_r<RetType, enums::enum_values<E>().size() ...>([&](auto ... Is) {
            return std::invoke(std::forward<Visitor>(visitor), enum_tag<enums::enum_values<E>()[Is]> ...);
        }, indexof(values) ...);
    }

    template<typename Visitor, enumeral E>
    decltype(auto) visit_enum(Visitor &&visitor, E value) {
        using return_type = std::invoke_result_t<Visitor, enum_tag_t<enums::enum_values<E>()[0]>>;
        return visit_enum<return_type>(std::forward<Visitor>(visitor), value);
    }

    template<enumeral E, std::predicate<E> auto F>
    constexpr size_t count_values_if() {
        size_t count = 0;
        for (E value : enum_values<E>()) {
            if (std::invoke(F, value)) {
                ++count;
            }
        }
        return count;
    }

    template<enumeral E, std::predicate<E> auto F>
    constexpr auto filtered_enum_array() {
        std::array<E, count_values_if<E, F>()> result;
        auto ptr = result.begin();
        for (E value : enum_values<E>()) {
            if (std::invoke(F, value)) {
                *ptr = value;
                ++ptr;
            }
        }
        return result;
    }
    
}

namespace json {

    template<enums::enumeral T, typename Context>
    struct deserializer<T, Context> {
        static T read(const json &value) {
            if (!value.IsString()) {
                throw deserialize_error(std::format("Cannot deserialize {}: value is not a string", reflect::type_name<T>()));
            }
            std::string_view str{value.GetString(), value.GetStringLength()};
            if (auto ret = enums::from_string<T>(str)) {
                return *ret;
            } else {
                throw deserialize_error(std::format("Invalid {} value: {}", reflect::type_name<T>(), str));
            }
        }
    };

    template<enums::enumeral T, typename Context>
    struct serializer<T, Context> {
        static void write(const T &value, string_writer &writer) {
            serialize(enums::to_string(value), writer);
        }
    };

}

#endif