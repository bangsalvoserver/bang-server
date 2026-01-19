#include "reverend.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_reverend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_play_card>(target_card, [=](player_ptr origin, card_ptr origin_card, const effect_context &ctx) -> game_string {
            if (origin_card->pocket == pocket_type::player_hand
                && origin_card->has_tag(tag_type::beer)
            ) {
                return {"ERROR_CARD_DISABLED_BY", origin_card, target_card};
            }
            return {};
        });
    }
}