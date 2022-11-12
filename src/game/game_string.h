#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "card_enums.h"

namespace banggame {

    DEFINE_STRUCT(card_format_id,
        (std::string, name)
        (card_sign, sign),

        card_format_id() = default;
        card_format_id(not_null<card *> value);
        card_format_id(card *value) : card_format_id(not_null{value}) {}
    )

    using game_format_arg = std::variant<int, std::string, card_format_id, serial::player>;
    
    DEFINE_STRUCT(game_string,
        (std::string, format_str)
        (std::vector<game_format_arg>, format_args),

        game_string() = default;
    
        game_string(
                std::convertible_to<std::string> auto &&message,
                auto && ... args)
            : format_str(FWD(message))
            , format_args{game_format_arg(FWD(args)) ...} {}

        explicit operator bool() const {
            return !format_str.empty();
        }
    )
}

#endif