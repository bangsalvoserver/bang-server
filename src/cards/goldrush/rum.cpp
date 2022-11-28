#include "rum.h"

#include "game/game.h"

namespace banggame {

    game_string effect_rum::on_prompt(card *origin_card, player *origin) {
        if (origin->m_hp == origin->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_rum::on_play(card *origin_card, player *origin) {
        std::vector<card_suit> suits;

        int num_cards = 3 + origin->get_num_checks();
        for (int i=0; i < num_cards; ++i) {
            card *drawn_card = origin->m_game->top_of_deck();
            suits.push_back(origin->get_card_sign(drawn_card).suit);
            origin->m_game->add_log("LOG_REVEALED_CARD", origin, drawn_card);
            origin->m_game->move_card(drawn_card, pocket_type::selection);
        }
        while (!origin->m_game->m_selection.empty()) {
            card *drawn_card = origin->m_game->m_selection.front();
            origin->m_game->call_event<event_type::on_draw_check_resolve>(origin, drawn_card);
            if (drawn_card->pocket == pocket_type::selection) {
                origin->m_game->move_card(drawn_card, pocket_type::discard_pile, nullptr);
            }
        }
        std::sort(suits.begin(), suits.end());
        origin->heal(int(std::unique(suits.begin(), suits.end()) - suits.begin()));
    }

}