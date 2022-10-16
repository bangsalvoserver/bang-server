#include "mustang.h"

#include "../../game.h"

namespace banggame {
    
    static bool is_horse(const card *c) {
        return c->has_tag(tag_type::horse);
    }

    game_string effect_horse::on_prompt(player *origin, card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            return {"PROMPT_REPLACE", target_card, *it};
        } else {
            return {};
        }
    }

    void effect_horse::on_equip(card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            target->discard_card(*it);
        }
    }

    void effect_mustang::on_enable(card *target_card, player *target) {
        ++target->m_distance_mod;
        target->send_player_status();
    }

    void effect_mustang::on_disable(card *target_card, player *target) {
        --target->m_distance_mod;
        target->send_player_status();
    }
}