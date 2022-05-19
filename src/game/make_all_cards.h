#ifndef __MAKE_ALL_CARDS_H__
#define __MAKE_ALL_CARDS_H__

#include <filesystem>

#include "card_data.h"

namespace banggame {

    struct all_cards_t {
        std::vector<card_data> deck;
        std::vector<card_data> characters;
        std::vector<card_data> goldrush;
        std::vector<card_data> hidden;
        std::vector<card_data> specials;
        std::vector<card_data> highnoon;
        std::vector<card_data> fistfulofcards;
        std::vector<card_data> wildwestshow;

        all_cards_t();
    };
}

#endif