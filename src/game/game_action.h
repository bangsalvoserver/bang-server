#ifndef __GAME_ACTION_H__
#define __GAME_ACTION_H__

#include "utils/reflector.h"

#include "card_enums.h"

namespace banggame {

    struct pick_card_args {REFLECTABLE(
        (pocket_type) pocket,
        (serial::player) player,
        (serial::card) card
    )};

    using target_vector = std::vector<play_card_target>;

    struct play_card_args {REFLECTABLE(
        (serial::card) card,
        (std::vector<serial::card>) modifiers,
        (target_vector) targets
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