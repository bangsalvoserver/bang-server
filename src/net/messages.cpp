#include "messages.h"

#include "utils/json_aggregate.h"
#include "cards/vtables.h"

namespace json {

    template<typename Context> struct serializer<const banggame::ruleset_vtable *, Context> {
        json operator()(const banggame::ruleset_vtable *value) const {
            return value->name;
        }
    };

}

namespace banggame {
    
    json::json serialize_message(const server_message &message) {
        return json::serialize(message);
    }

    client_message deserialize_message(const json::json &value) {
        return json::deserialize<client_message>(value);
    }

}