#include "blood_brothers.h"

#include "game/game.h"

#include "effects/base/can_play_card.h"

namespace banggame {

    void equip_blood_brothers::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, 2}, [=](player *origin) {
            target->m_game->queue_request<request_can_play_card>(target_card, nullptr, origin);
        });
    }

    game_string effect_blood_brothers::get_error(card *origin_card, player *origin, player *target) {
        if (origin->m_hp <= 1) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        if (target->m_hp == target->m_max_hp) {
            return {"ERROR_PLAYER_IS_FULL_HP", target};
        }
        return {};
    }

    void effect_blood_brothers::on_play(card *origin_card, player *origin, player *target) {
        origin->damage(origin_card, origin, 1);
        origin->m_game->queue_action([=]{ target->heal(1); });
    }

}