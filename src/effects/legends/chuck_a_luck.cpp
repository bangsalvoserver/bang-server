#include "chuck_a_luck.h"

#include "effects/base/steal_destroy.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

namespace banggame {

    void equip_chuck_a_luck::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr origin, card_ptr target_card, bool is_destroyed, bool &handled) {
            if (origin == target->m_game->m_playing && is_destroyed && target_card->has_tag(tag_type::bangcard)) {
                queue_request_perform_feat(origin_card, origin);
            }
        });
    }
}