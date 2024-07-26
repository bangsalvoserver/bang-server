#include "hangover.h"

#include "game/game.h"

namespace banggame {

    void equip_hangover::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_disabler(target_card, [](const_card_ptr c) {
            return c->pocket == pocket_type::player_character;
        });
    }

    void equip_hangover::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_disablers(target_card);
    }
}