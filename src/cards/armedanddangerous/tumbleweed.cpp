#include "tumbleweed.h"

#include "game/game.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct request_tumbleweed : request_base, resolvable_request {
        request_tumbleweed(card *origin_card, player *origin, player *target, card *drawn_card, card *drawing_card)
            : request_base(origin_card, origin, target)
            , target_card(drawing_card)
            , drawn_card(drawn_card) {}
        
        card *target_card;
        card *drawn_card;

        struct timer_tumbleweed : request_timer {
            explicit timer_tumbleweed(request_tumbleweed *request)
                : request_timer(request, request->target->m_game->m_options.tumbleweed_timer) {}
        
            void on_finished() override {
                static_cast<request_tumbleweed *>(request)->on_finished();
            }
        };

        timer_tumbleweed m_timer{this};
        request_timer *timer() override { return &m_timer; }

        std::vector<card *> get_highlights() const override {
            return {target_card, drawn_card};
        }

        void on_finished() {
            target->m_game->m_current_check.resolve(drawn_card);
        }

        void on_resolve() override {
            target->m_game->pop_request();
            on_finished();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_TUMBLEWEED", origin, origin_card, target_card, drawn_card};
            } else {
                return {"STATUS_CAN_PLAY_TUMBLEWEED_OTHER", origin, origin_card, target_card, drawn_card, target};
            }
        }
    };

    void equip_tumbleweed::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_check_select>(target_card, [=](player *origin, card *origin_card, card *drawn_card, bool &auto_resolve) {
            target->m_game->queue_request_front<request_tumbleweed>(target_card, origin, target, drawn_card, origin_card);
            auto_resolve = false;
        });
    }

    bool effect_tumbleweed::on_check_target(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<request_tumbleweed>();
        return (req->origin && bot_suggestion::target_friend{}.on_check_target(origin_card, origin, req->origin))
            != origin->m_game->m_current_check.check(req->drawn_card);
    }

    bool effect_tumbleweed::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_tumbleweed>(origin) != nullptr;
    }

    void effect_tumbleweed::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->m_game->m_current_check.restart();
    }

}