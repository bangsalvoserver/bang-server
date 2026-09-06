#ifndef __GAME_STATS_H__
#define __GAME_STATS_H__

#include <string>
#include <vector>
#include <cstdint>

#include "cards/card_defs.h"

namespace banggame {

    struct player_tracking {
        int bangs_played = 0;
        int ability_uses = 0;
        int dynamite_explosions = 0;
        int prison_turns_skipped = 0;
        int duels_lost = 0;
        int kills = 0;
    };

    struct player_game_report {
        int user_id = 0;
        std::string username;
        bool is_bot = false;
        std::string character;
        player_role role = player_role::unknown;
        bool survived = false;
        bool won = false;
        player_tracking stats;
    };

    struct game_report {
        int game_id = 0;
        int lobby_id = 0;
        int64_t started_at = 0;
        int64_t ended_at = 0;
        int num_players = 0;
        int num_rounds = 0;
        std::vector<std::string> expansions;
        std::vector<player_game_report> players;
    };

}

#endif
