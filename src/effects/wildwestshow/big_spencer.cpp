#include "big_spencer.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_big_spencer::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_play_card>(target_card, [=](player_ptr origin, card_ptr origin_card, const effect_context &ctx) -> game_string {
            if (origin == target
                && origin_card->pocket == pocket_type::player_hand
                && origin_card->has_tag(tag_type::missedcard)
            ) {
                return {"ERROR_CARD_DISABLED_BY", origin_card, target_card};
            }
            return {};
        });
    }

    void equip_initial_cards::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_initial_cards>(target_card, [=, ncards=ncards](const_player_ptr origin) -> std::optional<int> {
            if (origin == target) {
                return ncards;
            }
            return std::nullopt;
        });
    }
}