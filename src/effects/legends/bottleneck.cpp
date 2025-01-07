#include "bottleneck.h"

#include "perform_feat.h"

#include "effects/base/steal_destroy.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_bottleneck::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr origin, card_ptr target_card, bool &handled) {
            if (origin == target->m_game->m_playing && target_card->has_tag(tag_type::beer)) {
                queue_request_perform_feat(origin_card, target);
            }
        });
    }
}