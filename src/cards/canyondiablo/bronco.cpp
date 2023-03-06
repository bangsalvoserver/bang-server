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
            target->m_game->move_card(*it, pocket_type::button_row);
        }
    }

    void equip_bronco::on_unequip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_button_row, is_discard_bronco);
        if (it != target->m_game->m_button_row.end()) {
            target->m_game->move_card(*it, pocket_type::hidden_deck);
        }
    }
}