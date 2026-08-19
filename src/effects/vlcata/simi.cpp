#include "simi.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_simi_dynamite_master::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_damage_response>(target_card,
            [target](player_ptr origin, player_ptr victim, int &damage) {
                if (victim == target && origin && origin->name == "DYNAMITE") {
                    damage = 0;
                }
            });
    }

    void effect_simi_take_dynamite::on_play(card_ptr origin_card, player_ptr origin) {
        auto &discards = origin->m_game->m_discards;
        auto it = std::find_if(discards.begin(), discards.end(), [](card_ptr c){ return c->name == "DYNAMITE"; });
        if (it != discards.end()) {
            card_ptr dyn = *it;
            origin->equip_card(dyn);
        }
    }
}