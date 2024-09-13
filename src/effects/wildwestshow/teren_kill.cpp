#include "teren_kill.h"

#include "game/game.h"

#include "effects/base/deathsave.h"
#include "effects/base/draw_check.h"

namespace banggame {
    
    void equip_teren_kill::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_player_death>(origin_card, [=](player_ptr target, bool tried_save) {
            if (origin == target && !tried_save) {
                origin->m_game->queue_request<request_check>(origin, origin_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                    if (result) {
                        origin->set_hp(1);
                        origin->draw_card(1, origin_card);
                    }
                });
            }
        });
    }
}