#ifndef __GAME_OPTIONS_H__
#define __GAME_OPTIONS_H__

#include "durations.h"
#include "cards/card_defs.h"

#include "utils/enum_bitset.h"

namespace banggame {
    
    using namespace std::chrono_literals;
    
    struct game_options {
        struct keep_default_values{};

        enums::bitset<expansion_type> expansions;
        bool enable_ghost_cards = false;
        bool character_choice = true;
        bool quick_discard_all = true;
        int scenario_deck_size = 12;
        int num_bots = 0;
        game_duration damage_timer = 1500ms;
        game_duration escape_timer = 3000ms;
        game_duration bot_play_timer = 500ms;
        game_duration tumbleweed_timer = 3000ms;
        float duration_coefficient = 1.f;
        unsigned int game_seed = 0;
    };

}

#endif