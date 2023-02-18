#include "don_bell.h"

#include "game/game.h"

namespace banggame {

    void equip_don_bell::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_turn_end>({target_card, -2}, [=](player *target, bool skipped) {
            if (!skipped && p == target && !target->check_player_flags(player_flags::extra_turn)) {
                target->m_game->queue_action([target, target_card] {
                    target->m_game->draw_check_then(target, target_card, &card_sign::is_red, [=](bool result) {
                        if (result) {
                            target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                            ++target->m_extra_turns;
                        }
                    });
                });
            }
        });
    }
}