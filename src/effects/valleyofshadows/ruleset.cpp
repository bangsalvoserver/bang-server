#include "ruleset.h"

#include "escape.h"

#include "effects/base/damage.h"
#include "effects/base/escapable.h"
#include "effects/base/death.h"
#include "effects/ghost_cards/ruleset.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    void ruleset_valleyofshadows::on_apply(game_ptr game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr,
            [](card_ptr origin_card, player_ptr origin, const_player_ptr target, effect_flags flags, const escapable_request &req, escape_type &value) {
                if (!target->empty_hand()
                    && effect_escape::can_escape(origin, origin_card, flags)
                    && !rn::contains(target->m_game->m_discards, "ESCAPE", &card::name)
                ) {
                    value = escape_type::escape_timer;
                }
            });

        game->add_listener<event_type::check_damage_response>(nullptr, [](player_ptr target, bool &value) {
            if (!value && rn::any_of(target->m_game->m_players, [&](player_ptr p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(target->m_game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
        
        if (game->m_options.expansions.contains(GET_RULESET(ghost_cards))) {
            game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) { value = false; });
        }
    }

    void ruleset_udolistinu::on_apply(game_ptr game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr,
            [](card_ptr origin_card, player_ptr origin, const_player_ptr target, effect_flags flags, const escapable_request &req, escape_type &value) {
                if (!target->empty_hand()
                    && effect_escape2::can_escape(origin, origin_card, flags)
                    && !rn::contains(target->m_game->m_discards, "ESCAPE", &card::name)
                ) {
                    value = escape_type::escape_timer;
                }
            });
        
        game->add_listener<event_type::check_damage_response>(nullptr, [](player_ptr target, bool &value) {
            if (!value && rn::any_of(target->m_game->m_players, [&](player_ptr p) {
                return p != target && p != target->m_game->m_playing && p->alive() && !p->empty_hand();
            }) && !rn::contains(target->m_game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
        
        if (game->m_options.expansions.contains(GET_RULESET(ghost_cards))) {
            game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) { value = false; });
        }
    }

    bool ruleset_udolistinu::is_valid_with(const expansion_set &set) {
        return !set.contains(GET_RULESET(valleyofshadows));
    }
}