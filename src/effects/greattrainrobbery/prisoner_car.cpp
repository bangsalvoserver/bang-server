#include "prisoner_car.h"

#include "game/game_table.h"

namespace banggame {

    void equip_prisoner_car::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_immunity_modifier>(origin_card, [=](card_ptr e_origin_card, player_ptr e_origin, const_player_ptr e_target, effect_flags flags, card_list &cards) {
            if (e_origin_card && e_origin != e_target && e_target == origin && (e_origin_card->name == "DUEL" || e_origin_card->name == "INDIANS")) {
                cards.emplace_back(origin_card);
            }
        });
    }
}