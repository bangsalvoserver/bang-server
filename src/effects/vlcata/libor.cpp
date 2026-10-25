#include "libor.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"
#include "effects/base/bang.h"
#include "effects/base/damage.h"

#include "game/game_table.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer) || c->name == "WHISKY" || c->name == "TEQUILA";
    }

    void equip_libor::on_enable(card_ptr target_card, player_ptr target) {
        auto first_bang = std::make_shared<bool>(true);
        auto hit_confirmed = std::make_shared<bool>(false);
        auto played_alcohol = std::make_shared<bool>(false);

        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [first_bang, hit_confirmed, played_alcohol, target](player_ptr origin) {
                if (origin == target) {
                    *first_bang = true;
                    *hit_confirmed = false;
                    *played_alcohol = false;
                }
            });

        target->m_game->add_listener<event_type::apply_bang_modifier>(target_card,
            [first_bang, target, target_card](player_ptr origin, shared_request_bang req) {
                if (origin == target && target->m_game->m_playing == target && *first_bang) {
                    *first_bang = false;
                    target_card->flash_card();
                    req->bang_damage = 2;
                }
            });

        target->m_game->add_listener<event_type::on_hit>(target_card,
            [hit_confirmed, target](card_ptr origin_card, player_ptr origin, player_ptr hit_target, int damage, effect_flags flags) {
                if (origin == target && damage == 2 && flags.check(effect_flag::is_bang)) {
                    *hit_confirmed = true;
                }
            });

        target->m_game->add_listener<event_type::on_play_card>(target_card,
            [played_alcohol, target](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
                if (origin == target && is_alcohol_card(played_card)) {
                    *played_alcohol = true;
                }
            });

        target->m_game->add_listener<event_type::on_turn_end>(target_card,
            [hit_confirmed, played_alcohol, target, target_card](player_ptr origin, bool skipped) {
                if (origin == target && !skipped && *hit_confirmed && !*played_alcohol) {
                    target->damage(target_card, target, 1);
                }
            });
    }

}
