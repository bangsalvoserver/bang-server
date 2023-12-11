#include "teren_kill.h"

#include "game/game.h"

#include "cards/base/deathsave.h"

namespace banggame {
    
    void equip_teren_kill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>(origin_card, [=](player *target, bool tried_save) {
            if (origin == target && !tried_save) {
                origin->m_game->draw_check_then(origin, origin_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                    if (result) {
                        origin->set_hp(1);
                        origin->draw_card(1, origin_card);
                    }
                });
            }
        });
    }
}