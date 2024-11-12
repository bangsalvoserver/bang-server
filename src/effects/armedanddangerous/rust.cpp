#include "rust.h"

#include "game/game.h"
#include "game/game_options.h"

#include "effects/base/resolve.h"

namespace banggame {

    static void resolve_rust(card_ptr origin_card, player_ptr origin, player_ptr target) {
        for (card_ptr c : target->cube_slots() | rn::to_vector) {
            c->move_cubes(origin->first_character(), 1);
        }
    }

    struct request_rust : request_resolvable {
        request_rust(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags, 0) {}

        struct rust_timer : request_timer {
            explicit rust_timer(request_rust *request)
                : request_timer(request, request->target->m_game->m_options.escape_timer) {}
            
            void on_finished() override {
                resolve_rust(request->origin_card, request->origin, request->target);
            }
        };

        std::optional<rust_timer> m_timer{this};
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                switch (target->can_escape(origin, origin_card, flags)) {
                case 0:
                    auto_resolve();
                    break;
                case 2:
                    m_timer.reset();
                }
            }
        }

        void on_resolve() override {
            origin->m_game->pop_request();
            resolve_rust(origin_card, origin, target);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_RUST", origin_card};
            } else {
                return {"STATUS_RUST_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_rust::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        origin->m_game->queue_request<request_rust>(origin_card, origin, target, flags);
    }

}