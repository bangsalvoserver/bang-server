#include "al_preacher.h"

#include "game/game_table.h"

#include "effects/base/can_play_card.h"

namespace banggame {

    void equip_al_preacher::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_equip_card>(target_card, [=](player_ptr origin, player_ptr target, card_ptr equipped_card, const effect_context &ctx) {
            if (p != origin && (equipped_card->is_blue() || equipped_card->is_orange())) {
                p->m_game->queue_request<request_can_play_card>(target_card, origin, p);
            }
        });
    }

}