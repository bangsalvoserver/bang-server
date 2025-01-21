#include "sid_ketchum.h"

#include "effects/base/can_play_card.h"
#include "effects/base/heal.h"

#include "game/game_table.h"

namespace banggame {

    struct request_sid_ketchum_legend : request_can_play_card {
        using request_can_play_card::request_can_play_card;

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_SID_KETCHUM_LEGEND", origin_card};
            } else {
                return {"STATUS_SID_KETCHUM_LEGEND_OTHER", origin_card, target};
            }
        }
    };

    void equip_sid_ketchum_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_heal>(target_card, [=](player_ptr origin) {
            if (origin == target && target->m_game->m_playing == target) {
                target->m_game->queue_request<request_sid_ketchum_legend>(target_card, target, target);
            }
        });
    }

    bool effect_sid_ketchum_legend_free_bang::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_sid_ketchum_legend>(target_is{origin}) != nullptr;
    }

    void effect_sid_ketchum_legend_free_bang::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}