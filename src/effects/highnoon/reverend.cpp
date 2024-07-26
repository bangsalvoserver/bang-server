#include "reverend.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_reverend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_disabler(target_card, [](const_card_ptr c) {
            return c->pocket == pocket_type::player_hand
                && c->has_tag(tag_type::beer);
        });
    }

    void equip_reverend::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_disablers(target_card);
    }
}