#include "messages.h"

#include "cards/expansion_set.h"
#include "cards/vtables.h"

#include "utils/json_aggregate.h"

namespace banggame {
    
    std::string serialize_message(const server_message &message) {
        return json::serialize(message).dump(-1, ' ', true, nlohmann::json::error_handler_t::replace);
    }

    client_message deserialize_message(std::string_view value) {
        return json::deserialize<client_message>(json::json::parse(value));
    }

}