#include "weapon.h"

#include "game/game.h"
#include "game/prompts.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    struct is_weapon {
        const_card_ptr target_card;
        bool operator ()(const_card_ptr c) const {
            return c != target_card && c->has_tag(tag_type::weapon);
        }
    };

    game_string equip_weapon::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (range == 0) {
            MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
            return prompts::prompt_target_self(origin_card, origin, target);
        } else if (!origin->is_bot() && target == origin && origin->get_weapon_range() != 0) {
            if (auto it = rn::find_if(target->m_table, is_weapon{origin_card}); it != target->m_table.end()) {
                return {"PROMPT_REPLACE", origin_card, *it};
            }
        }
        return {};
    }

    void equip_weapon::on_enable(card_ptr target_card, player_ptr target, equip_flags flags) {
        if (!flags.check(equip_flag::disabler)) {
            if (auto it = rn::find_if(target->m_table, is_weapon{target_card}); it != target->m_table.end()) {
                target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, *it);
                target->discard_card(*it);
            }

            target->m_game->add_listener<event_type::count_range_mod>(target_card, [=, range=range](const_player_ptr origin, range_mod_type type, int &value) {
                if (origin == target && type == range_mod_type::weapon_range && !origin->m_game->is_disabled(target_card)) {
                    value = range;
                }
            });
        }
    }

    void equip_weapon::on_disable(card_ptr target_card, player_ptr target, equip_flags flags) {
        if (!flags.check(equip_flag::disabler)) {
            target->m_game->remove_listeners(target_card);
        }
    }
}