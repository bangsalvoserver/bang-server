#ifndef __GAME_STATS_DB_H__
#define __GAME_STATS_DB_H__

#include <string>
#include <string_view>
#include <optional>
#include <vector>

#include "game/game_stats.h"

namespace game_stats {

    void init(const std::string &db_file);

    int get_next_game_id();

    void save_game(const banggame::game_report &report);

    std::optional<banggame::game_report> get_game(int game_id);

    std::vector<banggame::game_report> search_games(std::string_view username, std::optional<int> lobby_id, size_t limit, size_t offset);

}

#endif
