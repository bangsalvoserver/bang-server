#include "for_a_few_cards_more.h"

#include "perform_feat.h"

#include "effects/base/requests.h"

#include "game/game.h"

namespace banggame {

    void equip_for_a_few_cards_more::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::post_discard_pass>(origin_card, [=](player_ptr origin, int ndiscarded) {
            if (origin == target->m_game->m_playing) {
                queue_request_perform_feat(origin_card, origin);
            }
        });
    }
}