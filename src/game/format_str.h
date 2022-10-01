#ifndef __FORMAT_STR_H__
#define __FORMAT_STR_H__

#include "card_enums.h"

#include <stdexcept>
#include <optional>

namespace banggame {

    struct card_format_id {REFLECTABLE(
        (std::string) name,
        (card_sign) sign
    )};

    using game_format_arg = std::variant<int, std::string, card_format_id, serial::player>;
    
    struct game_string {
        REFLECTABLE(
            (std::string) format_str,
            (std::vector<game_format_arg>) format_args
        )

        game_string() = default;
    
        template<std::convertible_to<std::string> T, typename ... Ts>
        game_string(T &&message, Ts && ... args);

        explicit operator bool() const {
            return !format_str.empty();
        }
    };
}

#endif