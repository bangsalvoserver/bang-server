#include "johnny_kisch.h"

#include "game/game.h"

namespace banggame {

    void equip_johnny_kisch::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_equip_card>(target_card, [=](player *origin, player *target, card *equipped_card, const effect_context &ctx) {
            if (p == origin) {
                for (player *other : range_other_players(target)) {
                    if (card *card = other->find_equipped_card(equipped_card)) {
                        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", target_card, other, card);
                        other->discard_card(card);
                    }
                }
            }
        });
    }
}