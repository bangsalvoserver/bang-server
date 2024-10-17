#include "doc_holyday.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    game_string handler_doc_holyday::on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void handler_doc_holyday::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        for (card_ptr c : target_cards) {
            effect_discard{}.on_play(origin_card, origin, c);
        }
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