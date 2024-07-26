#include "apache_kid.h"

#include "game/game.h"

namespace banggame {

    void equip_apache_kid::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card_ptr origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, card_list &cards) {
            if (origin_card && e_origin != e_target && e_target == target && target->m_game->get_card_sign(origin_card).is_diamonds()) {
                cards.emplace_back(target_card);
            }
        });
    }
}