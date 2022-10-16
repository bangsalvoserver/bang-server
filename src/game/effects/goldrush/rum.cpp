#include "rum.h"

#include "../../game.h"

namespace banggame {

    void effect_rum::on_play(card *origin_card, player *origin) {
        std::vector<card_suit> suits;

        int num_cards = 3 + origin->get_num_checks();
        for (int i=0; i < num_cards; ++i) {
            origin->m_game->add_log("LOG_REVEALED_CARD", origin, origin->m_game->m_deck.back());
            suits.push_back(origin->get_card_sign(origin->m_game->draw_card_to(pocket_type::selection, nullptr)).suit);
        }
        while (!origin->m_game->m_selection.empty()) {
            card *drawn_card = origin->m_game->m_selection.front();
            origin->m_game->call_event<event_type::on_draw_check>(origin, drawn_card);
            if (drawn_card->pocket == pocket_type::selection) {
                origin->m_game->move_card(drawn_card, pocket_type::discard_pile, nullptr);
            }
        }
        std::sort(suits.begin(), suits.end());
        origin->heal(static_cast<int>(std::unique(suits.begin(), suits.end()) - suits.begin()));
    }

}