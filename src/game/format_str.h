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

    struct player_format_id {REFLECTABLE(
        (int) player_id
    )};

    using game_format_arg = std::variant<int, std::string, card_format_id, player_format_id>;
    
    struct game_string {
        REFLECTABLE(
            (std::string) format_str,
            (std::vector<game_format_arg>) format_args
        )

        game_string() = default;
    
        template<std::convertible_to<std::string> T, typename ... Ts>
        game_string(T &&message, Ts && ... args);
    };

    using opt_game_str = std::optional<game_string>;
}

#endif