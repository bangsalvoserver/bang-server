#include "ladyrosaoftexas.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    static player_ptr get_swapping_player(player_ptr origin) {
        if (origin->m_game->check_flags(game_flag::invert_rotation)) {
            return origin->get_next_player();
        } else {
            return origin->get_prev_player();
        }
    }

    game_string effect_ladyrosaoftexas::get_error(card_ptr origin_card, player_ptr origin) {
        if (origin->m_game->num_alive() <= 2) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        } else {
            return {};
        }
    }

    game_string effect_ladyrosaoftexas::on_prompt(card_ptr origin_card, player_ptr origin) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, get_swapping_player(origin)));
        return {};
    }

    void effect_ladyrosaoftexas::on_play(card_ptr origin_card, player_ptr origin) {
        player_ptr target = get_swapping_player(origin);
        target->add_player_flags(player_flag::skip_turn);
        std::iter_swap(
            rn::find(origin->m_game->m_players, origin),
            rn::find(origin->m_game->m_players, target));
        origin->m_game->add_update(game_updates::player_order{ origin->m_game->m_players });
    }
}