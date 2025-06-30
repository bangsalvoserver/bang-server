#include "cactus.h"

#include "game/game_table.h"

#include "effects/base/draw_check.h"

namespace banggame {

    void effect_cactus::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, [=](card_ptr target_card) {
            return draw_check_result {
                .lucky = get_modified_sign(target_card).is_red(),
                .indifferent = origin->is_ghost() || origin->m_hp == origin->m_max_hp
            };
        }, [=](bool result) {
            if (result) {
                origin->heal(1);
            }
        });
    }
}