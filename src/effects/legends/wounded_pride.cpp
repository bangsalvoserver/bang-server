#include "wounded_pride.h"

#include "cards/game_enums.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void equip_wounded_pride::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_missed>(origin_card, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
            if (req->origin == target->m_game->m_playing && flags.check(effect_flag::is_bang)) {
                queue_request_perform_feat(origin_card, req->origin);
            }
        });
    }
}