#ifndef __PLAY_VERIFY_H__
#define __PLAY_VERIFY_H__

#include "game_update.h"

namespace banggame {

    DEFINE_ENUM_TYPES(message_type,
        (ok)
        (error, game_string)
        (prompt, game_string)
    )

    using game_message = enums::enum_variant<message_type>;

    game_string get_play_card_error(player *origin, card *origin_card, const effect_context &ctx);

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx);

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx);

    game_message verify_and_pick(player *origin, const pick_card_args &args);
    
    game_message verify_and_play(player *origin, const play_card_args &args);

}

#endif