#include "messages.h"

#include "cards/expansion_set.h"
#include "cards/vtables.h"

#include "utils/json_aggregate.h"

namespace json {

    template<typename Context>
    struct deserializer<banggame::client_messages::game_action, Context> {
        banggame::client_messages::game_action operator()(const json &value, const Context &ctx) const {
            return { value };
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
            
            constexpr auto type_names = std::array{ reflect::type_name<Ts>() ... };
            auto it = rn::find(type_names, key);
            if (it == rn::end(type_names)) {
                throw deserialize_error(std::format("Invalid variant type: {}", key));
            }
            size_t index = rn::distance(rn::begin(type_names), it);
            const json &inner_value = key_it.value();

            constexpr auto vtable = [&]<size_t ... Is>(std::index_sequence<Is...>) {
                return std::array{
                    +[](const json &inner_value, const Context &ctx) -> variant_type {
                        using type = std::variant_alternative_t<Is, variant_type>;
                        return deserialize_unchecked<type>(inner_value, ctx);
                    } ...
                };
            }(std::index_sequence_for<Ts...>());

            return vtable[index](inner_value, ctx);
        }
    };

    template<typename Context>
    struct serializer<banggame::server_messages::game_update, Context> {
        json operator()(const banggame::server_messages::game_update &value, const Context &ctx) const {
            return serialize_unchecked(value.update, ctx);
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

}

namespace banggame {
    
    std::string serialize_message(const server_message &message) {
        return json::serialize(message).dump(-1, ' ', true, nlohmann::json::error_handler_t::replace);
    }

    client_message deserialize_message(const json::json &value) {
        return json::deserialize<client_message>(value);
    }

}