#include "train_to_yooma.h"

#include "perform_feat.h"

#include "effects/base/draw_check.h"

#include "game/game.h"

namespace banggame {

    void equip_train_to_yooma::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_check_start>(origin_card, [=](player_ptr origin, shared_request_check req, bool &handled) {
            queue_request_perform_feat(origin_card, target->m_game->m_playing, 35);
        });
    }
}