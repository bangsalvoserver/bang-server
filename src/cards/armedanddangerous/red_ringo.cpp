#include "red_ringo.h"

#include "game/game.h"

namespace banggame {
    
    void equip_red_ringo::on_equip(card *target_card, player *target) {
        target->add_cubes(target->first_character(), max_cubes);
    }

    game_string handler_red_ringo::verify(card *origin_card, player *origin, const target_list &targets) {
        if (origin->first_character()->num_cubes < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin->first_character()};
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
            origin->move_cubes(origin->first_character(), target.get<target_type::card>(), 1);
        }
    }
}