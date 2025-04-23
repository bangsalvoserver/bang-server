#ifndef __GAME_OPTIONS_H__
#define __GAME_OPTIONS_H__

#include "durations.h"
#include "cards/card_fwd.h"

namespace banggame {
    
    using namespace std::chrono_literals;
    
    struct game_options {
        expansion_set expansions;
        int character_choice = 1;
        bool quick_discard_all = true;
        bool auto_pick_predraw = true;
        bool allow_bot_rejoin = false;
        bool only_base_characters = false;
        int scenario_deck_size = 12;
        int num_bots = 0;
        game_duration auto_resolve_timer = 1000ms;
        game_duration damage_timer = 1500ms;
        game_duration escape_timer = 3000ms;
        game_duration bot_play_timer = 500ms;
        float duration_coefficient = 1.f;
        unsigned int game_seed = 0;
        unsigned int bot_rng_seed = 0;

        static game_options deserialize_json(const json::json &value);

        std::string to_string(std::string_view sep = "\n") const;
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