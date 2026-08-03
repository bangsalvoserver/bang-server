#include "companion.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

#include "ruleset.h"

namespace banggame {

    void equip_companion::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_play_card>(target_card, [=](player_ptr origin, card_ptr origin_card, const effect_context &ctx) -> game_string {
            if (origin == target && ctx.contains<contexts::distance_start>()) {
                if (origin_card->pocket == pocket_type::player_hand && !origin_card->is_brown()) {
                    return {"ERROR_TARGET_NOT_BROWN_CARD", target_card, origin_card};
                }
            }
            return {};
        });
    }

    void equip_companion::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners({ target_card, 0 });
    }

    bool modifier_companion::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        if (player_ptr tracked_player = get_tracked_player(origin_card)) {
            return tracked_player->alive()
                && (playing_card->has_tag(tag_type::play_as_bang)
                || (playing_card->has_tag(tag_type::ranged_effect) && playing_card->is_brown()));
        }
        return false;
    }

    void modifier_companion::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        if (player_ptr target = get_tracked_player(origin_card)) {
            ctx.add(contexts::distance_start{ target });
        }
    }
}