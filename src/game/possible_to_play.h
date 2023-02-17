#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__


#include "player.h"

#include "game.h"

#include "play_verify.h"

namespace banggame {

    template<typename Rng>
    inline bool contains_at_least(Rng &&range, int size) {
        return ranges::distance(ranges::take_view(FWD(range), size)) == size;
    }

    ranges::any_view<player *> make_equip_set(player *origin, card *origin_card);

    ranges::any_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {});

    ranges::any_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {});

    bool is_possible_to_play_effects(player *origin, card *origin_card, const effect_list &effects, const effect_context &ctx = {});
    
    bool is_possible_to_play(player *origin, card *origin_card, bool is_response = false, const effect_context &ctx = {});
    
}

#endif