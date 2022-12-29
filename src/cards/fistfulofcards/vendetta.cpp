#include "vendetta.h"

#include "game/game.h"

namespace banggame {

    void equip_vendetta::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_turn_end>({target_card, -1}, [target_card](player *target, bool skipped) {
            if (!skipped && !target->check_player_flags(player_flags::extra_turn)) {
                target->m_game->queue_action([target, target_card] {
                    target->m_game->draw_check_then(target, target_card, [](card_sign sign) {
                        return sign.suit == card_suit::hearts;
                    }, [=](bool result) {
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