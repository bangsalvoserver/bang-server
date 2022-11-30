#include "youl_grinner.h"

#include "game/game.h"

namespace banggame {

    struct request_youl_grinner : request_base {
        request_youl_grinner(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target) {}

        bool auto_resolve() override {
            return auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", target, origin, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", target, origin);
            origin->steal_card(target_card);
            target->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
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
                for (player &p : range_other_players(target)) {
                    if (p.m_hand.size() > target->m_hand.size()) {
                        target->m_game->queue_request<request_youl_grinner>(target_card, target, &p);
                    }
                }
            }
        });
    }
}