#include "youl_grinner.h"

#include "game/game.h"

namespace banggame {

    bool request_youl_grinner::can_pick(const card *target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_youl_grinner::on_pick(card *target_card) {
        target->m_game->pop_request();
        if (target_card->visibility != card_visibility::shown) {
            target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", target, origin, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", target, origin);
        } else {
            target->m_game->add_log("LOG_GIFTED_CARD", target, origin, target_card);
        }
        origin->steal_card(target_card);
    }

    game_string request_youl_grinner::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_YOUL_GRINNER", origin_card, origin};
        } else {
            return {"STATUS_YOUL_GRINNER_OTHER", origin_card, origin, target};
        }
    }

    void equip_youl_grinner::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            if (target == origin) {
                for (player *p : range_other_players(target)) {
                    if (p->m_hand.size() > target->m_hand.size()) {
                        target->m_game->queue_request<request_youl_grinner>(target_card, target, p);
                    }
                }
            }
        });
    }
}