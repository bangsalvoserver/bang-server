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

        static void write(const variant_type &value, string_writer &writer, const Context &ctx) {
            std::visit([&](const auto &inner_value) {
                writer.StartObject();

                auto key = reflect::type_name<std::remove_cvref_t<decltype(inner_value)>>();
                writer.Key(key.data(), key.size());

                serialize(inner_value, writer, ctx);

                writer.EndObject();
            }, value);
        }
    };

    template<typename Context, typename ... Ts>
    struct deserializer<std::variant<Ts ...>, Context> {
        using variant_type = std::variant<Ts ...>;
        
        static variant_type read(const json &value, const Context &ctx) {
            if (!value.IsObject()) {
                throw deserialize_error("Cannot deserialize tagged variant: value is not an object");
            }
            if (value.MemberCount() != 1) {
                throw deserialize_error("Cannot deserialize tagged variant: object must contain only one key");
            }

            using deserialize_fun = variant_type (*)(const json &inner_value, const Context &ctx);
            static constexpr auto vtable = utils::make_static_map<std::string_view, deserialize_fun>({
                {
                    reflect::type_name<Ts>(),
                    [](const json &inner_value, const Context &ctx) -> variant_type {
                        return deserialize<Ts>(inner_value, ctx);
                    }
                } ...
            });

            auto key_it = value.MemberBegin();
            std::string_view key{key_it->name.GetString(), key_it->name.GetStringLength()};

            auto it = vtable.find(key);
            if (it == vtable.end()) {
                throw deserialize_error(std::format("Invalid variant type: {}", key));
            }

            return it->second(key_it->value, ctx);
        }
    };
}

#endif