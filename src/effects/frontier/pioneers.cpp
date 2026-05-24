#include "pioneers.h"

#include "ruleset.h"

#include "cards/game_events.h"

#include "effects/base/equip.h"

#include "game/game_table.h"

namespace banggame {

    game_string effect_equip_on_next::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        return effect_equip_on{}.get_error(origin_card, origin, origin->get_next_player(), ctx);
    }

    void effect_equip_on_next::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        effect_equip_on{}.add_context(origin_card, origin, origin->get_next_player(), ctx);
    }

    prompt_string effect_equip_on_next::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        return effect_equip_on{}.on_prompt(origin_card, origin, origin->get_next_player(), ctx);
    }

    void effect_equip_on_next::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        effect_equip_on{}.on_play(origin_card, origin, origin->get_next_player(), ctx);
    }

    void equip_pioneers::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({ target_card, 5 }, [=](player_ptr origin) {
            if (origin == target) {
                if (get_tracked_player(target_card) == target) {
                    target->discard_card(target_card);
                    int ncards = rn::count_if(target->m_game->m_players, [&](player_ptr p) { return p->alive() && p != target; });
                    target->draw_card(ncards, target_card);
                } else {
                    for (player_ptr dest : target->m_game->range_other_players(target)) {
                        if (!dest->find_equipped_card(target_card)) {
                            target->disable_equip(target_card);
                            dest->equip_card(target_card);
                            break;
                        }
                    }
                }
            }
        });
    }

    void equip_pioneers::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners({ target_card, 5 });
    }
}