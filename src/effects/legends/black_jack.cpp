#include "black_jack.h"

#include "game/game_table.h"

namespace banggame {

    int effect_black_jack_legend::get_card_rank_value(card_rank rank) {
        switch (rank) {
        case card_rank::rank_2:     return 2;
        case card_rank::rank_3:     return 3;
        case card_rank::rank_4:     return 4;
        case card_rank::rank_5:     return 5;
        case card_rank::rank_6:     return 6;
        case card_rank::rank_7:     return 7;
        case card_rank::rank_8:     return 8;
        case card_rank::rank_9:     return 9;
        case card_rank::rank_10:    return 10;
        case card_rank::rank_J:     return 10;
        case card_rank::rank_Q:     return 10;
        case card_rank::rank_K:     return 10;
        case card_rank::rank_A:     return 11;
        default:                    return 0;
        }
    }

    void effect_black_jack_legend::on_play(card_ptr origin_card, player_ptr origin) {
        int sum = 0;
        while (sum <= 21) {
            card_ptr drawn_card = origin->m_game->top_of_deck();
            origin->m_game->add_log("LOG_DRAWN_CARD", origin, drawn_card);
            drawn_card->set_visibility(card_visibility::shown);
            drawn_card->add_short_pause();
            origin->add_to_hand(drawn_card);
            sum += get_card_rank_value(drawn_card->sign.rank);
        }
    }
}