#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"

namespace banggame {

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx);

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx);

    game_string verify_and_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers);

}

#endif