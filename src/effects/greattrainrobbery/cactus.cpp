#include "cactus.h"

#include "game/game_table.h"

#include "effects/base/draw_check.h"

namespace banggame {

    void effect_cactus::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, [=](card_ptr target_card) {
            if (origin->is_ghost() || origin->m_hp == origin->m_max_hp) {
                return draw_check_result::indifferent;
            } else if (get_modified_sign(target_card).is_red()) {
                return draw_check_result::lucky;
            } else {
                return draw_check_result::unlucky;
            }
        }, [=](bool result) {
            if (result) {
                origin->heal(1);
            }
        });
    }
}