#ifndef __BANG_CARDS_H__
#define __BANG_CARDS_H__

#include "card_data.h"

namespace banggame {

    using sound_list = std::span<const sound_id>;

    struct expansion_data {
        ruleset_ptr expansion;
        sound_list sounds;
    };

    using expansion_map = utils::static_map_view<std::string_view, expansion_data>;

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

        expansion_map expansions;
    };

    extern const bang_cards_t bang_cards;

    inline card_data_list get_card_deck(card_deck_type deck) {
        switch (deck) {
            case card_deck_type::none:              return bang_cards.hidden;
            case card_deck_type::main_deck:         return bang_cards.deck;
            case card_deck_type::character:         return bang_cards.characters;
            case card_deck_type::goldrush:          return bang_cards.goldrush;
            case card_deck_type::highnoon:          return bang_cards.highnoon;
            case card_deck_type::fistfulofcards:    return bang_cards.fistfulofcards;
            case card_deck_type::wildwestshow:      return bang_cards.wildwestshow;
            case card_deck_type::station:           return bang_cards.stations;
            case card_deck_type::locomotive:        return bang_cards.locomotive;
            case card_deck_type::train:             return bang_cards.train;
            case card_deck_type::legends:           return bang_cards.legends;
            case card_deck_type::feats:             return bang_cards.feats;
            default: return {};
        }
    }
    
}

#endif