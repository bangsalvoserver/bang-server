#ifndef __GAME_OPTIONS_H__
#define __GAME_OPTIONS_H__

#include "durations.h"
#include "cards/card_defs.h"

#include "utils/enum_bitset.h"

namespace banggame {
    
    struct game_options {
        enums::bitset<expansion_type> expansions;
        bool enable_ghost_cards;
        bool character_choice;
        bool quick_discard_all;
        int scenario_deck_size;
        int num_bots;
        game_duration damage_timer;
        game_duration escape_timer;
        game_duration bot_play_timer;
        game_duration tumbleweed_timer;
        float duration_coefficient;
        unsigned int game_seed;
    };

    extern const game_options default_game_options;

}

namespace json {
    banggame::game_options deserialize_game_options(const json &value);

    template<typename Context>
    struct deserializer<banggame::game_options, Context> {
        banggame::game_options operator()(const json &value, const Context &ctx) const {
            return deserialize_game_options(value);
        }
    };
}

#endif