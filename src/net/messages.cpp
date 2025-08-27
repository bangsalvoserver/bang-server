#include "messages.h"

#include "cards/expansion_set.h"
#include "cards/vtables.h"

#include "utils/json_aggregate.h"
#include "utils/tagged_variant.h"

namespace banggame {
    
    std::string serialize_message(const server_message &message) {
        return json::to_string(message);
    }

    client_message deserialize_message(std::string_view value) {
        return json::parse_string<client_message>(value);
    }

}