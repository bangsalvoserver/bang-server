#include "vendetta.h"

#include "effects/base/draw_check.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_vendetta::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_turn_end>({target_card, -1}, [=](player_ptr target, bool skipped) {
            if (!skipped && !target->check_player_flags(player_flag::extra_turn)) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_hearts, [=](bool result) {
                    if (result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        ++target->m_extra_turns;
                    }
                });
            }
        });
    }
}