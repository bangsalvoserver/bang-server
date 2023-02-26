#include "bronco.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    static bool is_discard_bronco(card *c) {
        return c->has_tag(tag_type::bronco);
    }

    void equip_bronco::on_equip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_hidden_deck, is_discard_bronco);
        if (it != target->m_game->m_hidden_deck.end()) {
            card *found_card = *it;
            target->m_game->move_card(found_card, pocket_type::button_row, nullptr, card_visibility::hidden, true);
            target->m_game->set_card_visibility(found_card, nullptr, card_visibility::shown, true);
        }
    }

    void equip_bronco::on_unequip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_button_row, is_discard_bronco);
        if (it != target->m_game->m_button_row.end()) {
            target->m_game->move_card(*it, pocket_type::hidden_deck, nullptr, card_visibility::hidden, true);
        }
    }
}