#include "emiliano.h"

#include "cards/game_enums.h"
#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void equip_emiliano::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
            if (flags.check(effect_flag::is_bang)) {
                auto draw_card = [=](card_ptr c) {
                    origin->m_game->queue_action([=]{
                        if (origin->alive() && c->pocket != pocket_type::player_hand) {
                            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, c);
                            c->add_short_pause();
                            origin->add_to_hand(c);
                        }
                    });
                };
                if (req->target == origin) {
                    draw_card(req->origin_card);
                } else if (req->origin == origin && flags.check(effect_flag::is_missed)) {
                    draw_card(missed_card);
                }
            }
        });
    }
}