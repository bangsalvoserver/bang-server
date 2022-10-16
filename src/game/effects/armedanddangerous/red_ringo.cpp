#include "red_ringo.h"

#include "../../game.h"

namespace banggame {
    
    void effect_red_ringo::on_equip(card *target_card, player *target) {
        target->add_cubes(target->m_characters.front(), max_cubes);
    }

    game_string handler_red_ringo::verify(card *origin_card, player *origin, const target_list &targets) {
        if (origin->m_characters.front()->num_cubes < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin->m_characters.front()};
        }
        for (const auto &target : targets) {
            card *target_card = target.get<target_type::card>();
            if (target_card->num_cubes >= max_cubes) {
                return {"ERROR_CARD_HAS_FULL_CUBES", target_card};
            }
        }
        return {};
    }

    void handler_red_ringo::on_play(card *origin_card, player *origin, const target_list &targets) {
        for (const auto &target : targets) {
            origin->move_cubes(origin->m_characters.front(), target.get<target_type::card>(), 1);
        }
    }
}