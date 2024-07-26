#include "ruleset.h"

#include "effects/base/damage.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void ruleset_valleyofshadows::on_apply(game *game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr,
            [=](card_ptr origin_card, player_ptr origin, const_player_ptr target, effect_flags flags, int &value) {
                if (!target->empty_hand() && flags.check(effect_flag::escapable) && !rn::contains(game->m_discards, "ESCAPE", &card::name)) {
                    value = 1;
                }
            });

        game->add_listener<event_type::check_damage_response>(nullptr, [=](player_ptr target, bool &value) {
            if (!value && rn::any_of(game->m_players, [target](player_ptr p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
    }
}