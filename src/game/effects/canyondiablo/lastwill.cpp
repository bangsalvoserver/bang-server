#include "lastwill.h"

#include "../../game.h"

namespace banggame {

    struct request_lastwill : request_base, resolvable_request {
        request_lastwill(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_resolve() override {
            target->m_game->pop_request();
            target->m_game->update_request();
        }
        
        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_LASTWILL", origin_card};
            } else {
                return {"STATUS_LASTWILL_OTHER", origin_card, target};
            }
        }
    };

    void effect_lastwill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>({origin_card, -1}, [=](player *target, bool tried_save) {
            if (origin == target) {
                target->m_game->queue_action_front([=]{
                    if (target->m_hp <= 0) {
                        origin->m_game->queue_request<request_lastwill>(origin_card, origin);
                    }
                });
            }
        });
    }

    bool effect_lastwill::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<request_lastwill>(origin);
    }

    void handler_lastwill::on_play(card *origin_card, player *origin, const target_list &targets) {
        player *target = targets[0].get<target_type::player>();

        origin->m_game->pop_request();

        for (auto c : targets | std::views::drop(1)) {
            card *chosen_card = c.get<target_type::card>();
            if (chosen_card->pocket == pocket_type::player_hand) {
                origin->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
                origin->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
            } else {
                origin->m_game->add_log("LOG_GIFTED_CARD", origin, target, chosen_card);
            }
            target->add_to_hand(chosen_card);
        }

        origin->m_game->update_request();
    }
}