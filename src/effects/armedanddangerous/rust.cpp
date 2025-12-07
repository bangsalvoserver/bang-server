#include "rust.h"

#include "game/game_table.h"
#include "game/game_options.h"

#include "effects/base/resolve.h"
#include "effects/base/escapable.h"

namespace banggame {

    struct request_rust : request_escapable_resolvable {
        request_rust(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_escapable_resolvable(origin_card, origin, target, flags, 0) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                switch (get_escape_type()) {
                case escape_type::no_escape:
                    auto_resolve();
                    break;
                case escape_type::escape_timer:
                    set_duration(target->m_game->m_options.escape_timer);
                    break;
                case escape_type::escape_no_timer:
                    // ignore
                    break;
                }
            }
        }

        card_list get_highlights(player_ptr owner) const override {
            return cube_slots(target)
                | rv::filter([](card_ptr c) { return c->num_cubes() != 0; })
                | rn::to<std::vector>();
        }

        void on_resolve() override {
            origin->m_game->pop_request();
            for (card_ptr c : cube_slots(target) | rn::to<std::vector>()) {
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