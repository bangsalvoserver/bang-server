#include "emiliano.h"

#include "cards/game_enums.h"
#include "effects/base/bang.h"

#include "game/game.h"

namespace banggame {

    void equip_emiliano::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr missed_card, effect_flags flags) {
            if (flags.check(effect_flag::is_bang)) {
                auto draw_card = [=](card_ptr c) {
                    p->m_game->queue_action([=]{
                        if (p->alive() && c->pocket != pocket_type::player_hand) {
                            p->m_game->add_log("LOG_STOLEN_SELF_CARD", p, c);
                            c->add_short_pause();
                            p->add_to_hand(c);
                        }
                    });
                };
                if (target == p) {
                    draw_card(origin_card);
                } else if (origin == p && flags.check(effect_flag::is_missed)) {
                    draw_card(missed_card);
                }
            }
        });
    }
}