#include "wyatt_earl.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_wyatt_earl::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [=](card_ptr origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, card_list &cards) {
            if (origin_card && e_target == target && flags.check(effect_flag::multi_target)) {
                cards.emplace_back(target_card);
            }
        });
    }
}