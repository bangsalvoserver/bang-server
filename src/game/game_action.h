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

    template<target_type E>
    struct id_target_transform {
        using type = void;
    };

    template<> struct id_target_transform<target_type::player> {
        using type = int;
    };

    template<> struct id_target_transform<target_type::card> {
        using type = int;
    };

    template<> struct id_target_transform<target_type::cards_other_players> {
        using type = std::vector<int>;
    };

    template<> struct id_target_transform<target_type::cube> {
        using type = std::vector<int>;
    };

    using play_card_target_ids = enums::enum_variant<target_type, id_target_transform>;

    struct play_card_args {REFLECTABLE(
        (int) card_id,
        (std::vector<int>) modifier_ids,
        (std::vector<play_card_target_ids>) targets
    )};

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, pick_card_args)
        (play_card, play_card_args)
        (respond_card, play_card_args)
        (prompt_respond, bool)
    )

    using game_action = enums::enum_variant<game_action_type>;
    
}

#endif