#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "card_fwd.h"
#include "utils/small_pod.h"

namespace banggame {
        
    struct card_format {
        banggame::card *card;
        card_format() = default;
        card_format(banggame::card *card) : card(card) {}
    };

    DEFINE_ENUM_TYPES(game_format_arg_type,
        (integer, int)
        (card, card_format)
        (player, banggame::player *)
    )

    using game_format_arg = enums::enum_variant<game_format_arg_type>;
    using format_str_type = small_string;
    using format_args_type = small_vector<game_format_arg>;
    
    struct game_string {
        format_str_type format_str;
        format_args_type format_args;

        game_string() = default;
    
        game_string(
                std::convertible_to<format_str_type> auto &&message,
                auto && ... args)
            : format_str(FWD(message))
            , format_args{game_format_arg(FWD(args)) ...} {}

        explicit operator bool() const {
            return !format_str.empty();
        }
    };
    
    #define MAYBE_RETURN(...) if (auto value_ = __VA_ARGS__) return value_
}

#endif