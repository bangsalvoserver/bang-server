#include "messages.h"

#include "cards/vtables.h"

namespace banggame {
    
    std::string serialize_message(const server_message &message) {
        return json::to_string(message);
    }

    client_message deserialize_message(std::string_view value) {
        return json::parse_string<client_message>(value);
    }

}