#include "mustang.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {
    
    static bool is_horse(const card *c) {
        return c->has_tag(tag_type::horse);
    }

    game_string equip_horse::on_prompt(card *origin_card, player *origin, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            return {"PROMPT_REPLACE", origin_card, *it};
        } else {
            return {};
        }
    }

    void equip_horse::on_equip(card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            target->discard_card(*it);
        }
    }

    void equip_mustang::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_range_mod>(target_card, [=](const player *origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::distance_mod) {
                ++value;
            }
        });
    }
}