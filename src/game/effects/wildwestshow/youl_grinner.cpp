#include "youl_grinner.h"

#include "../../game.h"

namespace banggame {

    struct request_youl_grinner : request_base {
        request_youl_grinner(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target) {}

        bool can_pick(pocket_type pocket, player *target_player, card *target_card) const override {
            return pocket == pocket_type::player_hand && target_player == target;
        }

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", target, origin, target_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", target, origin);
            origin->steal_card(target_card);
            target->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
            target->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_YOUL_GRINNER", origin_card};
            } else {
                return {"STATUS_YOUL_GRINNER_OTHER", target, origin_card};
            }
        }
    };

    void effect_youl_grinner::on_enable(card *target_card, player *target) {
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