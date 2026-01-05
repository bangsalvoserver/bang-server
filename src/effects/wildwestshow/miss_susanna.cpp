#include "miss_susanna.h"

#include "ruleset.h"

#include "effects/base/pass_turn.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static int get_count_played_cards(player_ptr origin) {
        int count = 0;
        origin->m_game->call_event(event_type::get_count_played_cards{ origin, count });
        return count;
    }

    void equip_miss_susanna::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::prompt_pass_turn>(target_card, [=, max_count=max_count](player_ptr origin, prompt_string &out_prompt) {
            int count = get_count_played_cards(origin);
            if (count < max_count) {
                out_prompt = {"PROMPT_MISS_SUSANNA", target_card, count, max_count};
            }
        });

        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=, max_count=max_count](player_ptr origin, bool skipped) {
            int count = get_count_played_cards(origin);
            if ((!skipped || count) && count < max_count) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}