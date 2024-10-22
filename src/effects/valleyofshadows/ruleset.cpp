#include "ruleset.h"

#include "effects/base/damage.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void ruleset_valleyofshadows::on_apply(game *game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr, [](card_ptr origin_card, player_ptr origin, const_player_ptr target, effect_flags flags, int &value) {
            if (!target->empty_hand()
                && flags.check(effect_flag::escapable)
                && !rn::contains(target->m_game->m_discards, "ESCAPE", &card::name)
            ) {
                value = 1;
            }
        });

        game->add_listener<event_type::check_damage_response>(nullptr, [](player_ptr target, bool &value) {
            if (!value && rn::any_of(target->m_game->m_players, [&](player_ptr p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(target->m_game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
    }

    void ruleset_udolistinu::on_apply(game *game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr, [](card_ptr origin_card, player_ptr origin, const_player_ptr target, effect_flags flags, int &value) {
            if (!target->empty_hand()
                && flags.check(effect_flag::escapable)
                && flags.check(effect_flag::single_target)
                && !flags.check(effect_flag::multi_target)
                && !rn::contains(target->m_game->m_discards, "ESCAPE", &card::name)
            ) {
                value = 1;
            }
        });
        
        game->add_listener<event_type::check_damage_response>(nullptr, [](player_ptr target, bool &value) {
            if (!value && rn::any_of(target->m_game->m_players, [&](player_ptr p) {
                return p != target && p != target->m_game->m_playing && p->alive() && !p->empty_hand();
            }) && !rn::contains(target->m_game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
    }

    bool ruleset_udolistinu::is_valid_with(const expansion_set &set) {
        return !set.contains(GET_RULESET(valleyofshadows));
    }
}