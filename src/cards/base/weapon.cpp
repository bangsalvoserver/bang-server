#include "weapon.h"

#include "game/game.h"
#include "prompts.h"
#include "cards/filter_enums.h"

namespace banggame {

    static bool is_weapon(card *c) {
        return c->has_tag(tag_type::weapon);
    }

    game_string equip_weapon::on_prompt(card *origin_card, player *origin, player *target) {
        if (range == 0) {
            return prompt_target_self{}.on_prompt(origin_card, origin, target);
        } else if (!origin->is_bot() && target == origin && origin->m_weapon_range != 0) {
            if (auto it = std::ranges::find_if(target->m_table, is_weapon); it != target->m_table.end()) {
                return {"PROMPT_REPLACE", origin_card, *it};
            }
        }
        return {};
    }

    void equip_weapon::on_equip(card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_weapon); it != target->m_table.end()) {
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, *it);
            target->discard_card(*it);
        }
    }

    void equip_weapon::on_enable(card *target_card, player *target) {
        target->m_weapon_range = range;
        target->send_player_status();
    }

    void equip_weapon::on_disable(card *target_card, player *target) {
        target->m_weapon_range = 1;
        target->send_player_status();
    }
}