#ifndef __GAME_OPTIONS_H__
#define __GAME_OPTIONS_H__

#include "durations.h"
#include "cards/card_fwd.h"

namespace banggame {
    
    struct game_options {
        expansion_set expansions;
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

        static const game_options default_game_options;
        static game_options deserialize_json(const json::json &value);

        std::string to_string() const;
        void set_option(std::string_view key, std::string_view value);
    };

}

namespace json {

    template<typename Context>
    struct deserializer<banggame::game_options, Context> {
        banggame::game_options operator()(const json &value, const Context &ctx) const {
            return banggame::game_options::deserialize_json(value);
        }
    };
}

#endif