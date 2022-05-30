#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "player.h"

namespace banggame{

    struct game_format_arg_visitor {
        game_format_arg operator()(int value) const {
            return value;
        }

        game_format_arg operator()(std::string value) const {
            return std::move(value);
        }

        game_format_arg operator()(player *value) const {
            return player_format_id{value->id};
        }

        game_format_arg operator()(card *value) const {
            return card_format_id{value->name, value->sign};
        }
    };

    game_formatted_string::game_formatted_string(
            std::convertible_to<std::string> auto &&message,
            auto && ... args)
        : format_str(FWD(message))
        , format_args{game_format_arg_visitor{}(FWD(args)) ...} {}

}

#endif