#ifndef __TAGGED_VARIANT_H__
#define __TAGGED_VARIANT_H__

#include <variant>
#include <reflect>

#include "json_serial.h"
#include "static_map.h"

namespace json {

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

            using deserialize_fun = variant_type (*)(const json &inner_value, const Context &ctx);
            static constexpr auto vtable = utils::make_static_map<std::string_view, deserialize_fun>({
                {
                    reflect::type_name<Ts>(),
                    [](const json &inner_value, const Context &ctx) -> variant_type {
                        return deserialize_unchecked<Ts>(inner_value, ctx);
                    }
                } ...
            });

            auto key_it = value.begin();
            std::string_view key = key_it.key();

            auto it = vtable.find(key);
            if (it == vtable.end()) {
                throw deserialize_error(std::format("Invalid variant type: {}", key));
            }

            return it->second(key_it.value(), ctx);
        }
    };
}

#endif