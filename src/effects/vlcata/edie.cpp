#include "edie.h"

#include "effects/base/steal_destroy.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    void equip_edie::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_destroy_card>(target_card, [target, target_card](player_ptr origin, card_ptr origin_card, card_ptr stolen_card, destroy_flags &flags) {
            if (origin && origin != target && stolen_card->owner == target) {
                target->m_game->queue_request<request_check>(target, target_card, std::not_fn(&card_sign::is_spades), [target, target_card, origin](bool result) {
                    if (result && origin->alive() && !origin->empty_hand()) {
                        target_card->flash_card();
                        origin->discard_card(origin->random_hand_card());
                    }
                });
            }
        });
    }

}
