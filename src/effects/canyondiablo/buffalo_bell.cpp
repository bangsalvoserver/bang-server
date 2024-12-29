#include "buffalo_bell.h"

#include "effects/base/bang.h"
#include "effects/base/missed.h"
#include "effects/base/steal_destroy.h"

#include "effects/legends/black_jack.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    bool effect_buffalo_bell::can_play(card_ptr origin_card, player_ptr origin) {
        return effect_missed{}.can_play(origin_card, origin);
    }

    static int get_card_suit_sum(player_ptr origin, card_ptr target_card) {
        int sum = effect_black_jack_legend::get_card_rank_value(target_card->get_modified_sign().rank);
        card_ptr origin_card = origin->m_game->top_request()->origin_card;
        if (origin_card) {
            sum += effect_black_jack_legend::get_card_rank_value(origin_card->get_modified_sign().rank);
        }
        return sum;
    }

    game_string effect_buffalo_bell::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        int sum = get_card_suit_sum(origin, target_card);
        if (sum < 13) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_buffalo_bell::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        player_ptr shooter = origin->m_game->top_request()->origin;
        effect_discard{}.on_play(origin_card, origin, target_card);
        
        int sum = get_card_suit_sum(origin, target_card);
        if (sum >= 13) {
            effect_missed{}.on_play(origin_card, origin);
            if (sum >= 20) {
                if (shooter) {
                    origin->m_game->queue_request<request_bang>(origin_card, origin, shooter, effect_flag::single_target, 20);
                }
            } else if (sum >= 17) {
                origin->draw_card(1, origin_card);
            }
        }
    }
}