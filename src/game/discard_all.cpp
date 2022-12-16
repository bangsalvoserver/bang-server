#include "game.h"

namespace banggame {

    inline void discard_card(player *target, card *target_card) {
        target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
        target->discard_card(target_card);
    }

    struct request_discard_all : request_base, resolvable_request {
        discard_all_reason reason;

        request_discard_all(player *target, discard_all_reason reason = discard_all_reason::death)
            : request_base(nullptr, nullptr, target)
            , reason(reason) {}
        
        bool can_pick(card *target_card) const override {
            return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
                && target_card->owner == target
                && !target_card->is_black();
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                discard_card(target, target_card);
            });
        }

        void on_update() override {
            if (!sent) {
                target->untap_inactive_cards();
            }
            
            if (target->m_game->m_options.auto_discard_all
                || (std::ranges::count_if(target->m_table, std::not_fn(&card::is_black)) + target->m_hand.size()) <= 1)
            {
                on_resolve();
            }
        }

        void on_resolve() override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                for (card *c : to_vector(target->m_table | std::views::filter(std::not_fn(&card::is_black)))) {
                    discard_card(target, c);
                }
                while (!target->empty_hand()) {
                    discard_card(target, target->m_hand.front());
                }
                while (!target->m_table.empty()) {
                    discard_card(target, target->m_table.front());
                }
                target->drop_all_cubes(target->first_character());
                if (reason != discard_all_reason::sheriff_killed_deputy) {
                    target->add_gold(-target->m_gold);
                }
                if (reason == discard_all_reason::death) {
                    target->m_game->play_sound(nullptr, "death");
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (reason != discard_all_reason::sheriff_killed_deputy) {
                if (target == owner) {
                    return "STATUS_DISCARD_ALL";
                } else {
                    return {"STATUS_DISCARD_ALL_OTHER", target};
                }
            } else {
                if (target == owner) {
                    return "STATUS_SHERIFF_KILLED_DEPUTY";
                } else {
                    return {"STATUS_SHERIFF_KILLED_DEPUTY_OTHER", target};
                }
            }
        }
    };

    void player::discard_all(discard_all_reason reason) {
        m_game->queue_request<request_discard_all>(this, reason);
    }

}