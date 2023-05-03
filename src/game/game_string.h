#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "cards/card_enums.h"
#include "utils/small_pod.h"

namespace banggame {

    DEFINE_STRUCT(card_format,
        (std::string, name)
        (card_sign, sign)
    )

    DEFINE_ENUM_TYPES(game_format_arg_type,
        (integer, int)
        (card, serial::card_format)
        (player, serial::opt_player)
    )

    using game_format_arg = enums::enum_variant<game_format_arg_type>;

#ifdef BUILD_BANG_CLIENT
    using format_str_type = std::string;
    using format_args_type = std::vector<game_format_arg>;
#else
    using format_str_type = small_string;
    using format_args_type = small_vector<game_format_arg>;
#endif
    
    DEFINE_STRUCT(game_string,
        (format_str_type, format_str)
        (format_args_type, format_args),

        game_string() = default;
    
        game_string(
                std::convertible_to<format_str_type> auto &&message,
                auto && ... args)
            : format_str(FWD(message))
            , format_args{game_format_arg(FWD(args)) ...} {}

        explicit operator bool() const {
            return !format_str.empty();
        }
    )
    
    #define MAYBE_RETURN(...) if (auto value_ = __VA_ARGS__) return value_
}

#endif