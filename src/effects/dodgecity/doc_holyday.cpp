#include "doc_holyday.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    prompt_string effect_doc_holyday::on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        return effect_bang{}.on_prompt(origin_card, origin, target);
    }

    void effect_doc_holyday::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        if (!rn::all_of(target_cards, [&](card_ptr target_card) {
            return target->immune_to(target_card, origin, {}, true);
        })) {
            effect_bang{}.on_play(origin_card, origin, target);
        } else {
            for (card_ptr target_card : target_cards) {
                target->immune_to(target_card, origin, {});
            }
        }
    }
}