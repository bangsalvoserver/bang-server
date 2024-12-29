#include "kit_carlson.h"

#include "cards/game_enums.h"

#include "effects/base/draw.h"
#include "effects/base/resolve.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    struct request_kit_carlson_legend : request_resolvable {
        request_kit_carlson_legend(card_ptr origin_card, player_ptr origin)
            : request_resolvable(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<3; ++i) {
                    target->m_game->top_of_deck()->move_to(pocket_type::selection, target);
                }
            }
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();

            while (!target->m_game->m_selection.empty()) {
                card_ptr c = target->m_game->m_selection.front();
                if (!target->m_game->check_flags(game_flag::hands_shown)) {
                    target->m_game->add_log(update_target::includes(target), "LOG_DRAWN_CARD", target, c);
                    target->m_game->add_log(update_target::excludes(target), "LOG_DRAWN_CARDS", target, 1);
                } else {
                    target->m_game->add_log("LOG_DRAWN_CARD", target, c);
                }
                target->add_to_hand(c);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON_LEGEND", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_LEGEND_OTHER", target, origin_card};
            }
        }
    };

    void equip_kit_carlson_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, shared_request_draw req_draw, bool &handled) {
            if (!handled && origin == target) {
                target->m_game->queue_request<request_kit_carlson_legend>(target_card, target);
                handled = true;
            }
        });
    }

    bool effect_kit_carlson_legend_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_kit_carlson_legend>(target_is{origin}) != nullptr;
    }

    void effect_kit_carlson_legend_response::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}