#include "hangover.h"

#include "../../game.h"

namespace banggame {

    void effect_hangover::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_character;
        });
    }

    void effect_hangover::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->call_event<event_type::on_effect_end>(target, target_card);
    }
}