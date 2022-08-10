#ifndef __GAME_ACTION_H__
#define __GAME_ACTION_H__

#include "utils/reflector.h"

#include "card_enums.h"

namespace banggame {

    struct pick_card_args {REFLECTABLE(
        (pocket_type) pocket,
        (int) player_id,
        (int) card_id
    )};

    DEFINE_ENUM_VARIANT(play_card_target_id, target_type,
        (player,                int)
        (conditional_player,    int)
        (card,                  int)
        (extra_card,            int)
        (cards_other_players,   std::vector<int>)
        (cube,                  std::vector<int>)
    )

    struct play_card_args {REFLECTABLE(
        (int) card_id,
        (std::vector<int>) modifier_ids,
        (std::vector<play_card_target_id>) targets
    )};

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, pick_card_args)
        (play_card, play_card_args)
        (respond_card, play_card_args)
        (prompt_respond, bool)
        (request_confirm)
    )

    using game_action = enums::enum_variant<game_action_type>;
    
}

#endif