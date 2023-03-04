#include "prisoner_car.h"

#include "game/game.h"

namespace banggame {

    void equip_prisoner_car::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_immunity_modifier>(origin_card, [=](card *e_origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            if (e_origin_card && e_origin != e_target && e_target == origin && (e_origin_card->name == "DUEL" || e_origin_card->name == "INDIANS")) {
                origin->m_game->add_log("LOG_PLAYER_IMMUNE_TO_CARD", origin, e_origin_card, origin_card);
                origin->m_game->flash_card(origin_card);
                value = true;
            }
        });
    }
}