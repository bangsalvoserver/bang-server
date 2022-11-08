#include "damage.h"

#include "game/game.h"
#include "deathsave.h"

namespace banggame {
    
    game_string effect_damage::verify(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (origin == target && origin->m_hp <= damage) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        return {};
    }

    void effect_damage::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (!target->is_ghost()) {
            if (target->m_game->call_event<event_type::check_damage_response>(false)) {
                target->m_game->queue_request_front<timer_damaging>(origin_card, origin, target, damage, flags);
            } else {
                on_resolve(origin_card, origin, target, flags);
            }
        }
    }

    void effect_damage::on_resolve(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->add_log(damage == 1 ? "LOG_TAKEN_DAMAGE" : "LOG_TAKEN_DAMAGE_PLURAL", origin_card, target, damage);
        target->set_hp(target->m_hp - damage);
        target->m_game->call_event<event_type::before_hit>(origin_card, origin, target, damage, flags);
        if (target->m_hp <= 0) {
            target->m_game->queue_request<request_death>(origin_card, origin, target);
        }
        target->m_game->call_event<event_type::after_hit>(origin_card, origin, target, damage, flags);
    }

    std::vector<card *> timer_damaging::get_highlights() const {
        return target->m_backup_character;
    }

    void timer_damaging::on_finished() {
        effect_damage{damage}.on_resolve(origin_card, origin, target, flags);
    }

    game_string timer_damaging::status_text(player *owner) const {
        if (bool(flags & effect_flags::play_as_bang)) {
            if (damage > 1) {
                return {"STATUS_DAMAGING_AS_BANG_PLURAL", target, origin_card, damage};
            } else {
                return {"STATUS_DAMAGING_AS_BANG", target, origin_card};
            }
        } else {
            if (damage > 1) {
                return {"STATUS_DAMAGING_PLURAL", target, origin_card, damage};
            } else {
                return {"STATUS_DAMAGING", target, origin_card};
            }
        }
    }

}