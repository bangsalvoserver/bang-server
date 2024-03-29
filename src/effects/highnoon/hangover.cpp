#include "hangover.h"

#include "game/game.h"

namespace banggame {

    void equip_hangover::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_character;
        });
    }

    void equip_hangover::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
    }
}