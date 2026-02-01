#include "damage.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/bot_suggestion.h"

#include "death.h"

namespace banggame {
    
    game_string effect_damage::get_error(card_ptr origin_card, player_ptr origin, effect_flags flags) {
        if (origin->m_hp <= damage) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        return {};
    }

    game_string effect_damage::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->is_bot() && origin->m_hp <= origin->m_max_hp - 2) {
            return "BOT_TOO_MUCH_DAMAGE";
        }
        return {};
    }

    void effect_damage::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        bot_suggestion::signal_hostile_action(origin, target);
        
        target->damage(origin_card, origin, damage, flags);
    }

    request_damage::request_damage(card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags)
        : request_base(origin_card, origin, target, flags, 200)
        , damage(damage) {}
    
    card_list request_damage::get_highlights(player_ptr owner) const {
        return { target->get_character() };
    }

    void request_damage::on_update() {
        if (target->is_ghost()) {
            target->m_game->pop_request();
        } else if (target->m_game->call_event(event_type::check_damage_response{ target })) {
            set_duration(target->m_game->m_options.damage_timer);
        } else {
            on_finished();
        }
    }

    void request_damage::on_finished() {
        target->m_game->pop_request();
        
        if (flags.check(effect_flag::play_as_bang)) {
            if (flags.check(effect_flag::target_players)) {
                target->m_game->add_log("LOG_TAKEN_DAMAGE_AS_GATLING", origin_card, target);
            } else {
                target->m_game->add_log("LOG_TAKEN_DAMAGE_AS_BANG", origin_card, target, damage);
            }
        } else {
            target->m_game->add_log("LOG_TAKEN_DAMAGE", origin_card, target, damage);
        }
        target->set_hp(target->m_hp - damage);
        target->m_game->queue_request<request_death>(origin_card, origin, target);
        target->m_game->call_event(event_type::on_hit{ origin_card, origin, target, damage, flags });
    }

    game_string request_damage::status_text(player_ptr owner) const {
        if (flags.check(effect_flag::play_as_bang)) {
            if (flags.check(effect_flag::target_players)) {
                return {"STATUS_DAMAGING_AS_GATLING", target, origin_card};
            } else {
                return {"STATUS_DAMAGING_AS_BANG", target, origin_card, damage};
            }
        } else {
            return {"STATUS_DAMAGING", target, origin_card, damage};
        }
    }

}