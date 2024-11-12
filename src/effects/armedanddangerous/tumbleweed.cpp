#include "tumbleweed.h"

#include "game/game.h"
#include "game/game_options.h"

#include "effects/base/resolve.h"
#include "effects/base/draw_check.h"

namespace banggame {

    struct request_tumbleweed : request_resolvable {
        request_tumbleweed(card_ptr origin_card, player_ptr origin, player_ptr target, shared_request_check &&handler)
            : request_resolvable(origin_card, origin, target, {}, 120)
            , handler(std::move(handler)) {}

        shared_request_check handler;

        struct timer_tumbleweed : request_timer {
            explicit timer_tumbleweed(request_tumbleweed *request)
                : request_timer(request, request->target->m_game->m_options.tumbleweed_timer) {}
        
            void on_finished() override {
                static_cast<request_tumbleweed *>(request)->on_finished();
            }
        };

        timer_tumbleweed m_timer{this};
        request_timer *timer() override {
            if (m_timer.get_duration() <= ticks{}) {
                return nullptr;
            } else {
                return &m_timer;
            }
        }

        card_list get_highlights() const override {
            auto vec = handler->get_drawn_cards();
            vec.push_back(handler->get_drawing_card());
            return vec;
        }

        void on_finished() {
            handler->resolve();
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
            on_finished();
        }

        game_string status_text(player_ptr owner) const override {
            auto cards = handler->get_drawn_cards();
            card_ptr drawing_card = handler->get_drawing_card();
            if (cards.size() == 1) {
                card_ptr drawn_card = cards.front();
                if (target == owner) {
                    return {"STATUS_REQ_TUMBLEWEED", origin, origin_card, drawing_card, drawn_card};
                } else {
                    return {"STATUS_REQ_TUMBLEWEED_OTHER", origin, origin_card, drawing_card, drawn_card, target};
                }
            } else {
                if (target == owner) {
                    return {"STATUS_REQ_TUMBLEWEED_FOR", origin, origin_card, drawing_card};
                } else {
                    return {"STATUS_REQ_TUMBLEWEED_FOR_OTHER", origin, origin_card, drawing_card, target};
                }
            }
        }
    };

    void equip_tumbleweed::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_check_select>(target_card, [=](player_ptr origin, shared_request_check req, bool &handled) {
            target->m_game->queue_request<request_tumbleweed>(target_card, origin, target, std::move(req));
            handled = true;
        });
    }

    prompt_string effect_tumbleweed::on_prompt(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_tumbleweed>();
        return req->handler->redraw_prompt(origin_card, origin);
    }

    bool effect_tumbleweed::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_tumbleweed>(origin) != nullptr;
    }

    void effect_tumbleweed::on_play(card_ptr origin_card, player_ptr origin) {
        auto handler = origin->m_game->top_request<request_tumbleweed>()->handler;
        origin->m_game->pop_request();
        handler->restart();
    }

}