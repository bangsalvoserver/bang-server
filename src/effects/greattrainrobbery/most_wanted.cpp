#include "most_wanted.h"

#include "game/game_table.h"
#include "game/game_options.h"

#include "effects/base/resolve.h"
#include "effects/base/draw_check.h"
#include "effects/base/escapable.h"

namespace banggame {

    struct request_most_wanted : request_resolvable, escapable_request {
        request_most_wanted(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags) {}

        std::optional<resolve_timer> m_timer;
        request_timer *timer() override { return m_timer ? &*m_timer : nullptr; }
        
        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                switch (get_escape_type(origin, target, origin_card, flags)) {
                case escape_type::no_escape:
                    m_timer.emplace(target->m_game->m_options.auto_resolve_timer);
                    break;
                case escape_type::escape_timer:
                    m_timer.emplace(std::max(
                        target->m_game->m_options.auto_resolve_timer,
                        target->m_game->m_options.escape_timer
                    ));
                    break;
                }
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            target->m_game->queue_request<request_check>(target, origin_card, std::not_fn(&card_sign::is_spades),
                [origin_card=origin_card, origin=origin, target=target](bool result) {
                    if (!result) {
                        target->damage(origin_card, origin, 1);
                    }
                });
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_MOST_WANTED", origin_card};
            } else {
                return {"STATUS_MOST_WANTED_OTHER", target, origin_card};
            }
        }
    };

    void effect_most_wanted::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        origin->m_game->queue_request<request_most_wanted>(origin_card, origin, target, flags);
    }
}