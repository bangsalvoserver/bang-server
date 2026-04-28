#include "rust.h"

#include "game/game_table.h"

#include "effects/base/resolve.h"
#include "effects/base/escapable.h"

namespace banggame {

    struct request_rust : request_escapable {
        request_rust(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_escapable(origin_card, origin, target, flags, 0) {}

        card_list get_highlights(player_ptr owner) const override {
            return cube_slots(target)
                | rv::filter([](card_ptr c) { return c->num_cubes() != 0; })
                | rn::to<std::vector>();
        }

        void on_resolve() override {
            pop_request();
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