#include "rust.h"

#include "game/game.h"

namespace banggame {

    static void resolve_rust(card *origin_card, player *origin, player *target) {
        for (card *c : ranges::to<std::vector>(target->cube_slots())) {
            target->move_cubes(c, origin->first_character(), 1);
        }
    }

    struct request_rust : request_base, resolvable_request {
        request_rust(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : request_base(origin_card, origin, target, flags) {}

        struct rust_timer : request_timer {
            explicit rust_timer(request_rust *request)
                : request_timer(request, request->target->m_game->m_options.escape_timer) {}
            
            void on_finished() override {
                resolve_rust(request->origin_card, request->origin, request->target);
            }
        };

        rust_timer m_timer{this};
        request_timer *timer() override { return &m_timer; }

        void on_update() override {
            switch (target->can_escape(origin, origin_card, flags)) {
            case 0:
                auto_respond();
                break;
            case 2:
                m_timer.disable();
            }
        }

        void on_resolve() override {
            origin->m_game->invoke_action([&]{
                origin->m_game->pop_request();
                resolve_rust(origin_card, origin, target);
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_RUST", origin_card};
            } else {
                return {"STATUS_RUST_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_rust::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (!target->immune_to(origin_card, origin, flags)) {
            origin->m_game->queue_action([=]{
                origin->m_game->queue_request<request_rust>(origin_card, origin, target, flags);
            });
        }
    }

}