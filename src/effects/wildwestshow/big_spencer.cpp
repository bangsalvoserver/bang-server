#include "big_spencer.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    void equip_big_spencer::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_play_card>(target_card, [=](player_ptr origin, card_ptr origin_card, const effect_context &ctx, game_string &out_error) {
            if (origin == target
                && origin_card->pocket == pocket_type::player_hand
                && origin_card->has_tag(tag_type::missedcard)
            ) {
                out_error = {"ERROR_CARD_DISABLED_BY", origin_card, target_card};
            }
        });
    }
}