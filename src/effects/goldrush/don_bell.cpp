#include "don_bell.h"

#include "effects/base/draw_check.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_don_bell::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_turn_end>({target_card, -2}, [=](player_ptr target, bool skipped) {
            if (!skipped && p == target && !target->check_player_flags(player_flag::extra_turn)) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_red, [=](bool result) {
                    if (result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        ++target->m_extra_turns;
                    }
                });
            }
        });
    }
}