#include "teren_kill.h"

#include "../../game.h"

namespace banggame {
    
    void effect_teren_kill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>(origin_card, [=](player *target, bool tried_save) {
            if (origin == target && !tried_save) {
                origin->m_game->draw_check_then(origin, origin_card, [=](card *drawn_card) {
                    if (origin->get_card_sign(drawn_card).suit != card_suit::spades) {
                        origin->set_hp(1);
                        origin->draw_card(1, origin_card);
                    }
                });
            }
        });
    }
}