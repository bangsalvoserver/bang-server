#include "thejudge.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_thejudge::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_equip_card>({target_card, -10}, [](player_ptr origin, card_ptr origin_card, const_player_ptr target, const effect_context &ctx, game_string &out_error) {
            out_error = "ERROR_CANT_EQUIP_CARDS";
        });
    }
}