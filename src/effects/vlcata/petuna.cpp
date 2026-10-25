#include "petuna.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"
#include "effects/base/draw_check.h"
#include "effects/base/resolve.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/request_timer.h"

namespace banggame {

    struct request_petuna_check : request_resolvable, request_timer {
        request_petuna_check(card_ptr origin_card, player_ptr origin)
            : request_resolvable(origin_card, origin, origin) {}

        void on_update() override {
            set_duration(origin->m_game->m_options.auto_resolve_timer * 10);
        }

        void on_finished() override {
            pop_request();
        }

        prompt_string resolve_prompt() const override {
            if (origin->is_bot()) {
                return {};
            }
            return "PROMPT_PETUNA_CHECK";
        }

        void on_resolve() override {
            pop_request();

            player_ptr p = origin;
            card_ptr origin_card_ = origin_card;
            origin->m_game->queue_request<request_check>(p, origin_card_, [](card_sign sign) {
                return draw_check_result{ .lucky = sign.is_red() };
            }, [p, origin_card_](card_sign sign) {
                if (sign.is_spades()) {
                    origin_card_->flash_card();
                    p->damage(origin_card_, p, 1);
                } else if (sign.is_red()) {
                    origin_card_->flash_card();
                    ++p->m_extra_turns;
                }
            });
        }

        game_string status_text(player_ptr owner) const override {
            return "STATUS_PETUNA_CHECK";
        }
    };

    void equip_petuna::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target, target_card](player_ptr origin, bool skipped) {
            if (!skipped && origin == target && !target->check_player_flags(player_flag::extra_turn)) {
                target->m_game->queue_request<request_petuna_check>(target_card, target);
            }
        });
    }

}
