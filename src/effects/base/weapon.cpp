#include "weapon.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    static card_ptr find_weapon_card(card_ptr origin_card, player_ptr target) {
        for (card_ptr target_card : target->m_table) {
            if (origin_card != target_card && target_card->has_tag(tag_type::weapon)) {
                return target_card;
            }
        }
        return nullptr;
    };

    game_string equip_weapon::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (range == 0) {
            MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
            return prompts::prompt_target_self(origin_card, origin, target);
        } else if (target == origin && origin->get_weapon_range() != 0) {
            if (card_ptr target_card = find_weapon_card(origin_card, target)) {
                if (!target->is_bot() || origin_card->get_tag_value(tag_type::weapon) <= target_card->get_tag_value(tag_type::weapon)) {
                    return {"PROMPT_REPLACE", origin_card, target_card};
                }
            }
        }
        return {};
    }

    void equip_weapon::on_enable(card_ptr origin_card, player_ptr target) {
        if (card_ptr target_card = find_weapon_card(origin_card, target)) {
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
            target->discard_card(target_card);
        }

        target->m_game->add_listener<event_type::count_range_mod>(origin_card, [=, range=range](const_player_ptr origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::weapon_range && !origin->m_game->is_disabled(origin_card)) {
                value = range;
            }
        });
    }

    void equip_weapon::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }
}