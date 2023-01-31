#include "mustang.h"

#include "game/game.h"

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
        ++target->m_distance_mod;
        target->send_player_status();
    }

    void equip_mustang::on_disable(card *target_card, player *target) {
        --target->m_distance_mod;
        target->send_player_status();
    }
}