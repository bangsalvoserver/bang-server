#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__


#include "player.h"

#include "game.h"

#include "play_verify.h"

namespace banggame {

    inline auto make_equip_set(player *origin, card *origin_card) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                if (origin_card->self_equippable()) {
                    return origin == target;
                } else {
                    return !check_player_filter(origin, origin_card->equip_target, target)
                        && !target->find_equipped_card(origin_card);
                }
            });
    }

    inline auto make_player_target_set(player *origin, card *origin_card, effect_holder holder) {
        return origin->m_game->m_players
            | ranges::views::filter([=](player *target) {
                return !check_player_filter(origin, holder.player_filter, target)
                    && !holder.verify(origin_card, origin, target);
            });
    }

    inline auto make_card_target_set(player *origin, card *origin_card, effect_holder holder) {
        return make_player_target_set(origin, origin_card, holder)
            | ranges::views::transform([](player *target) {
                return ranges::views::concat(target->m_hand, target->m_table);
            })
            | ranges::views::join
            | ranges::views::filter([=](card *target_card) {
                return !check_card_filter(origin_card, origin, holder.card_filter, target_card)
                    && !holder.verify(origin_card, origin, target_card);
            });
    }

    bool is_possible_to_play(player *origin, card *target_card, const effect_list &effects);
    
}

#endif