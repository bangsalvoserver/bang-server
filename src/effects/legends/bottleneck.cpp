#include "bottleneck.h"

#include "effects/base/steal_destroy.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

namespace banggame {

    void equip_bottleneck::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr origin, card_ptr target_card, bool is_destroyed, destroy_flags &flags) {
            if (origin == target->m_game->m_playing && is_destroyed && target_card->has_tag(tag_type::beer)) {
                queue_request_perform_feat(origin_card, origin);
            }
        });
    }
}