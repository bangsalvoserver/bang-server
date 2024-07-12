#ifndef __CARD_FWD_H__
#define __CARD_FWD_H__

#include "utils/enum_variant.h"
#include "utils/small_pod.h"
#include "utils/utils.h"

namespace banggame {
    
    struct game;
    struct card;
    struct player;

    struct effect_context;
    
    struct effect_vtable;
    struct equip_vtable;
    struct modifier_vtable;
    struct mth_vtable;

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

}

namespace banggame::serial {

    using opt_card = banggame::card *;
    using card = not_null<opt_card>;
    using opt_player = banggame::player *;
    using player = not_null<opt_player>;
    using int_list = small_int_set;
    
    using player_list = std::vector<player>;
    using card_list = std::vector<card>;
    
}

#endif