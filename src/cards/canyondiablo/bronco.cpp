#include "bronco.h"

#include "game/game.h"

namespace banggame {

    static bool is_discard_bronco(card *c) {
        return c->has_tag(tag_type::bronco);
    }

    void equip_bronco::on_equip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_hidden_deck, is_discard_bronco);
        if (it != target->m_game->m_hidden_deck.end()) {
            card *found_card = *it;
            target->m_game->move_card(found_card, pocket_type::button_row, nullptr, show_card_flags::instant | show_card_flags::hidden);
            target->m_game->send_card_update(found_card, nullptr, show_card_flags::instant);
        }
    }

    void equip_bronco::on_unequip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_button_row, is_discard_bronco);
        if (it != target->m_game->m_button_row.end()) {
            target->m_game->move_card(*it, pocket_type::hidden_deck, nullptr, show_card_flags::instant | show_card_flags::hidden);
        }
    }
}