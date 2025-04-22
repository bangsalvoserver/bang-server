#ifndef __TAGGED_VARIANT_H__
#define __TAGGED_VARIANT_H__

#include <variant>
#include <reflect>

#include "json_serial.h"
#include "static_map.h"

namespace utils {

    template<typename T, typename Variant>
    struct find_variant_type_index;

    template<typename T, typename First, typename ... Ts>
    struct find_variant_type_index<T, std::variant<First, Ts ...>> {
        static constexpr size_t value = 1 + find_variant_type_index<T, std::variant<Ts ...>>::value;
    };

    template<typename T, typename ... Ts>
    struct find_variant_type_index<T, std::variant<T, Ts ...>> {
        static constexpr size_t value = 0;
    };
    
    template<typename Variant>
    class tagged_variant_index {
    private:
        size_t m_index;

    public:
        constexpr tagged_variant_index() = default;

        explicit constexpr tagged_variant_index(const Variant &variant)
            : m_index{variant.index()} {}

        template<typename T>
        explicit constexpr tagged_variant_index(std::in_place_type_t<T>)
            : m_index{find_variant_type_index<T, Variant>::value} {}
        
        explicit constexpr tagged_variant_index(std::string_view key) {
            static constexpr auto names_map = []<size_t ... Is>(std::index_sequence<Is ...>) {
                return utils::make_static_map<std::string_view, size_t>({
                    { reflect::type_name<std::variant_alternative_t<Is, Variant>>(), Is } ...
                });
            }(std::make_index_sequence<std::variant_size_v<Variant>>());

            auto it = names_map.find(key);
            if (it == names_map.end()) {
                throw std::runtime_error(std::format("Invalid variant type: {}", key));
            }
            m_index = it->second;
        }

        constexpr bool operator == (const tagged_variant_index &other) const = default;

        constexpr size_t index() const {
            return m_index;
        }

        constexpr std::string_view to_string() const {
            static constexpr auto names = []<size_t ... Is>(std::index_sequence<Is ...>) {
                return std::array{ reflect::type_name<std::variant_alternative_t<Is, Variant>>() ... };
            }(std::make_index_sequence<std::variant_size_v<Variant>>());

            return names[index()];
        }
    };

    template<typename RetType, typename Visitor, typename ... Ts>
    RetType visit_tagged(Visitor &&visitor, tagged_variant_index<std::variant<Ts ...>> index) {
        static constexpr std::array<RetType (*)(Visitor&&), sizeof...(Ts)> vtable {
            [](Visitor &&visitor) -> RetType {
                return std::invoke(std::forward<Visitor>(visitor), std::in_place_type<Ts>);
            } ...
        };
        return vtable[index.index()](std::forward<Visitor>(visitor));
    }

    template<typename Visitor, typename ... Ts>
    decltype(auto) visit_tagged(Visitor &&visitor, tagged_variant_index<std::variant<Ts ...>> index) {
        using return_type = std::invoke_result_t<Visitor, std::in_place_type_t<std::variant_alternative_t<0, std::variant<Ts ...>>>>;
        return visit_tagged<return_type>(std::forward<Visitor>(visitor), index);
    }
}

namespace json {

    template<typename Context, typename ... Ts>
    struct serializer<utils::tagged_variant_index<std::variant<Ts ...>>, Context> {
        json operator()(const utils::tagged_variant_index<std::variant<Ts ...>> &value) const {
            return value.to_string();
        }
    };

    template<typename Context, typename ... Ts>
    struct deserializer<utils::tagged_variant_index<std::variant<Ts ...>>, Context> {
        using value_type = utils::tagged_variant_index<std::variant<Ts ...>>;
        value_type operator()(const json &value) const {
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize tagged variant index: value is not a string");
            }
            return value_type{std::string_view(value.get<std::string>())};
        }
    };

    template<typename Context, typename ... Ts>
    struct serializer<std::variant<Ts ...>, Context> {
        using variant_type = std::variant<Ts ...>;

        json operator()(const variant_type &value, const Context &ctx) const {
            return std::visit([&](const auto &inner_value) {
                return json{{
                    reflect::type_name<std::remove_cvref_t<decltype(inner_value)>>(),
                    serialize_unchecked(inner_value, ctx)
                }};
            }, value);
        }
    };

    template<typename Context, typename ... Ts>
    struct deserializer<std::variant<Ts ...>, Context> {
        using variant_type = std::variant<Ts ...>;
        
        variant_type operator()(const json &value, const Context &ctx) const {
            if (!value.is_object()) {
                throw deserialize_error("Cannot deserialize tagged variant: value is not an object");
            }
            if (value.size() != 1) {
                throw deserialize_error("Cannot deserialize tagged variant: object must contain only one key");
            }

            auto key_it = value.begin();
            std::string_view key = key_it.key();

            utils::tagged_variant_index<variant_type> index{key};
            const json &inner_value = key_it.value();

            return utils::visit_tagged([&]<typename T>(std::in_place_type_t<T>) -> variant_type {
                return deserialize_unchecked<T>(inner_value, ctx);
            }, index);
        }
    };
}

#endif