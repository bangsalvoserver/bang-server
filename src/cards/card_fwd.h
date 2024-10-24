#ifndef __CARD_FWD_H__
#define __CARD_FWD_H__

#include "utils/enum_bitset.h"
#include "utils/small_int_set.h"
#include "utils/nullable.h"
#include "utils/misc.h"

#include <set>

namespace banggame {
    
    struct game;
    struct card;
    struct player;

    using card_ptr = card *;
    using player_ptr = player *;

    using const_card_ptr = const card *;
    using const_player_ptr = const player *;

    using card_list = std::vector<card_ptr>;
    using player_list = std::vector<player_ptr>;

    using nullable_card = utils::nullable<card_ptr>;
    using nullable_player = utils::nullable<player_ptr>;

    struct effect_context;
    struct game_options;
    
    struct effect_vtable;
    struct equip_vtable;
    struct modifier_vtable;
    struct mth_vtable;

    struct ruleset_vtable;

    using expansion_set = std::set<const ruleset_vtable *>;

    enum class target_player_filter;
    enum class target_card_filter;
    enum class tag_type;
    
    enum class effect_flag;
    enum class game_flag;
    enum class player_flag;
    enum class discard_all_reason;

    using effect_flags = enums::bitset<effect_flag>;
    using game_flags = enums::bitset<game_flag>;
    using player_flags = enums::bitset<player_flag>;

    struct game_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

}

#endif