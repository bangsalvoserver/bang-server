#include "rust.h"

#include "game/game.h"

namespace banggame {

    static void resolve_rust(card *origin_card, player *origin, player *target) {
        auto view = target->m_table | std::views::filter([](card *c){ return c->color == card_color_type::orange; });
        std::vector<card *> orange_cards{view.begin(), view.end()};
        
        orange_cards.push_back(target->m_characters.front());
        for (card *c : orange_cards) {
            target->move_cubes(c, origin->m_characters.front(), 1);
        }
    }

    struct request_rust : request_base, resolvable_request {
        request_rust(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        struct rust_timer : request_timer {
            explicit rust_timer(request_rust *request)
                : request_timer(request, request->target->m_game->m_options.escape_timer_ms) {}
            
            void on_finished() override {
                resolve_rust(request->origin_card, request->origin, request->target);
            }
        };

        rust_timer m_timer{this};
        request_timer *timer() override { return &m_timer; }

        bool auto_resolve() override {
            return (target->m_hand.empty() || !target->can_escape(origin, origin_card, flags))
                && auto_respond();
        }

        void on_resolve() override {
            auto lock = origin->m_game->lock_updates(true);
            resolve_rust(origin_card, origin, target);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_RUST", origin_card};
            } else {
                return {"STATUS_RUST_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_rust::on_prompt(card *origin_card, player *origin, player *target) {
        if (target->count_cubes() == 0) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }
    
    void effect_rust::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (target->count_cubes() == 0) return;
        origin->m_game->queue_action([=]{
            origin->m_game->queue_request<request_rust>(origin_card, origin, target, flags);
        });
    }

}