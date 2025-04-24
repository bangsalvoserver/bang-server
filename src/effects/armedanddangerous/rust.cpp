#include "rust.h"

#include "game/game_table.h"
#include "game/game_options.h"

#include "effects/base/resolve.h"
#include "effects/base/escapable.h"

namespace banggame {

    struct request_rust : request_resolvable, escapable_request {
        request_rust(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags, 0) {}

        std::optional<resolve_timer> m_timer;
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                switch (get_escape_type(origin, target, origin_card, flags)) {
                case escape_type::no_escape:
                    auto_resolve();
                    break;
                case escape_type::escape_timer:
                    m_timer.emplace(target->m_game->m_options.escape_timer);
                }
            }
        }

        void on_resolve() override {
            origin->m_game->pop_request();
            for (card_ptr c : cube_slots(target) | rn::to_vector) {
                c->move_cubes(origin->get_character(), 1);
            }
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