#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"

namespace banggame {

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx);

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx);

    std::pair<game_string, bool> verify_and_pick(player *origin, card *target_card);
    
    std::pair<game_string, bool> verify_and_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, bool bypass_prompt);

}

#endif