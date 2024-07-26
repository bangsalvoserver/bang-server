#include "indianguide.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_indianguide::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card_ptr origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, card_list &cards) {
            if (origin_card && e_target == target && origin_card->has_tag(tag_type::indians)) {
                cards.emplace_back(target_card);
            }
        });
    }
}