#include "ladyrosaoftexas.h"

#include "game/game.h"

namespace banggame {

    game_string effect_ladyrosaoftexas::verify(card *origin_card, player *origin) {
        if (origin->m_game->m_players.size() <= 2) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        } else {
            return {};
        }
    }

    void effect_ladyrosaoftexas::on_play(card *origin_card, player *origin) {
        auto next_player = player_iterator(origin);
        if (origin->m_game->check_flags(game_flags::invert_rotation)) {
            ++next_player;
        } else {
            --next_player;
        }
        (*next_player)->add_player_flags(player_flags::skip_turn);
        std::iter_swap(
            std::ranges::find(origin->m_game->m_players, origin),
            std::ranges::find(origin->m_game->m_players, *next_player));
        origin->m_game->add_update<game_update_type::player_order>(origin->m_game->m_players
            | ranges::views::filter(&player::alive)
            | ranges::to<std::vector<not_null<player *>>>);
    }
}