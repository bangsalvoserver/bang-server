#include "damage.h"

#include "../../game.h"
#include "deathsave.h"

namespace banggame {

    void effect_damage::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (!target->is_ghost()) {
            if (target->m_game->call_event<event_type::check_damage_response>(false)) {
                target->m_game->queue_request_front<timer_damaging>(origin_card, origin, target, amount, flags);
            } else {
                on_resolve(origin_card, origin, target, flags);
            }
        }
    }

    void effect_damage::on_resolve(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->add_log(amount == 1 ? "LOG_TAKEN_DAMAGE" : "LOG_TAKEN_DAMAGE_PLURAL", origin_card, target, amount);
        target->set_hp(target->m_hp - amount);
        target->m_game->call_event<event_type::before_hit>(origin_card, origin, target, amount, flags);
        if (target->m_hp <= 0) {
            target->m_game->queue_request<request_death>(origin_card, origin, target);
        }
        target->m_game->call_event<event_type::after_hit>(origin_card, origin, target, amount, flags);
    }
    
    game_string effect_self_damage::verify(card *origin_card, player *origin) {
        if (origin->m_hp <= 1) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        return {};
    }

    void effect_self_damage::on_play(card *origin_card, player *origin) {
        origin->damage(origin_card, origin, 1);
    }

    std::vector<card *> timer_damaging::get_highlights() const {
        return target->m_backup_character;
    }

    void timer_damaging::on_finished() {
        effect_damage{damage}.on_resolve(origin_card, origin, target, flags);
    }

    game_string timer_damaging::status_text(player *owner) const {
        return {damage > 1 ? "STATUS_DAMAGING_PLURAL" : "STATUS_DAMAGING", target, origin_card, damage};
    }

}