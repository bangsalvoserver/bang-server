#include "youl_grinner.h"

#include "game/game.h"

namespace banggame {

    struct request_youl_grinner : request_base {
        request_youl_grinner(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                if (target_card->visibility != card_visibility::shown) {
                    target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", target, origin, target_card);
                    target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", target, origin);
                } else {
                    target->m_game->add_log("LOG_GIFTED_CARD", target, origin, target_card);
                }
                origin->steal_card(target_card);
                target->m_game->call_event<event_type::on_use_hand_card>(target, target_card, false);
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_YOUL_GRINNER", origin_card, origin};
            } else {
                return {"STATUS_YOUL_GRINNER_OTHER", origin_card, origin, target};
            }
        }
    };

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