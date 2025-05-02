#ifndef __BANG_CARDS_H__
#define __BANG_CARDS_H__

#include "card_data.h"

namespace banggame {

    using sound_list = std::span<const sound_id>;

    struct expansion_data {
        ruleset_ptr expansion;
        sound_list sounds;
    };

    using expansion_data_list = std::span<const expansion_data>;

    using card_data_list = std::span<const card_data>;

    struct bang_cards_t {
        card_data_list deck;
        card_data_list characters;
        card_data_list goldrush;
        card_data_list highnoon;
        card_data_list fistfulofcards;
        card_data_list wildwestshow;
        card_data_list button_row;
        card_data_list stations;
        card_data_list train;
        card_data_list locomotive;
        card_data_list legends;
        card_data_list feats;
        card_data_list hidden;

        expansion_data_list expansions;
    };

    extern const bang_cards_t bang_cards;
    
}

#endif