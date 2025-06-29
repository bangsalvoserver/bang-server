#include "dining_car.h"

#include "game/game_table.h"

#include "effects/base/predraw_check.h"

namespace banggame {

    void equip_dining_car::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_predraw_check>(origin_card, [=](player_ptr target, card_ptr target_card) {
            if (origin == target && origin_card == target_card) {
                origin->m_game->queue_request<request_check>(origin, origin_card, [=](card_ptr target_card) {
                    return draw_check_result {
                        .lucky = get_modified_sign(target_card).is_hearts(),
                        .indifferent = origin->is_ghost() || origin->m_hp == origin->m_max_hp
                    };
                }, [=](bool result) {
                    if (result) {
                        origin->heal(1);
                    }
                });
            }
        });
    }
}