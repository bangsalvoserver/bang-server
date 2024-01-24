#include "buffalo_bell.h"

#include "cards/base/bang.h"
#include "cards/base/missed.h"
#include "cards/base/steal_destroy.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    game_string effect_buffalo_bell::get_error(card *origin_card, player *origin, card *target_card) {
        if (!effect_missed{}.can_play(origin_card, origin)) {
            return "ERROR_INVALID_ACTION";
        }
        return {};
    }

    static int get_card_suit_sum(player *origin, card *target_card) {
        int sum = static_cast<int>(origin->m_game->get_card_sign(target_card).rank) + 1;
        card *origin_card = origin->m_game->top_request()->origin_card;
        if (origin_card) {
            sum += static_cast<int>(origin->m_game->get_card_sign(origin_card).rank) + 1;
        }
        return sum;
    }

    game_string effect_buffalo_bell::on_prompt(card *origin_card, player *origin, card *target_card) {
        int sum = get_card_suit_sum(origin, target_card);
        if (sum < 13) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_buffalo_bell::on_play(card *origin_card, player *origin, card *target_card) {
        player *shooter = origin->m_game->top_request()->origin;
        effect_discard{}.on_play(origin_card, origin, target_card);
        
        int sum = get_card_suit_sum(origin, target_card);
        if (sum >= 13) {
            effect_missed{}.on_play(origin_card, origin);
            if (sum >= 20) {
                if (shooter) {
                    origin->m_game->queue_request<request_bang>(origin_card, origin, shooter, effect_flags::single_target, 20);
                }
            } else if (sum >= 17) {
                origin->draw_card(1, origin_card);
            }
        }
    }
}