#include "wilhelm_scream.h"

#include "perform_feat.h"

#include "effects/base/bang.h"

#include "game/game.h"

namespace banggame {

    void equip_wilhelm_scream::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr origin, shared_request_bang req) {
            if (origin == target->m_game->m_playing && origin->m_game->calc_distance(origin, req->target) + req->target->get_distance_mod() >= 2) {
                queue_request_perform_feat(origin_card, origin);
            }
        });
    }
}