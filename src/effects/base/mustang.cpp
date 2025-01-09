#include "mustang.h"

#include "game/game_table.h"
#include "cards/filter_enums.h"

namespace banggame {
    
    struct is_horse {
        const_card_ptr target_card;
        bool operator ()(const_card_ptr c) const {
            return c != target_card && c->has_tag(tag_type::horse);
        }
    };

    game_string equip_horse::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (auto it = rn::find_if(target->m_table, is_horse{origin_card}); it != target->m_table.end()) {
            return {"PROMPT_REPLACE", origin_card, *it};
        } else {
            return {};
        }
    }

    void equip_horse::on_enable(card_ptr target_card, player_ptr target) {
        if (auto it = rn::find_if(target->m_table, is_horse{target_card}); it != target->m_table.end()) {
            target->discard_card(*it);
        }
    }

    void equip_mustang::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_range_mod>(target_card, [=](const_player_ptr origin, range_mod_type type, int &value) {
            if (origin == target && type == range_mod_type::distance_mod) {
                ++value;
            }
        });
    }
}