#include "messages.h"

#include "cards/expansion_set.h"
#include "cards/vtables.h"

#include "utils/json_aggregate.h"

namespace banggame {
    
    json::json serialize_message(const server_message &message) {
        return json::serialize(message);
    }

    client_message deserialize_message(const json::json &value) {
        return json::deserialize<client_message>(value);
    }

}