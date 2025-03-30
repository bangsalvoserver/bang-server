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

    template<typename Context>
    struct serializer<banggame::server_messages::game_update, Context> {
        json operator()(const banggame::server_messages::game_update &value, const Context &ctx) const {
            return serialize_unchecked(value.update, ctx);
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