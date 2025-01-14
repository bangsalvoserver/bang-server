#include "vulture_sam.h"

#include "effects/base/death.h"

#include "game/game_table.h"

namespace banggame {

    static card_ptr find_base_character(card_ptr origin_card) {
        for (card_ptr c : origin_card->m_game->m_characters) {
            if (c->name == origin_card->name) {
                return c;
            }
        }
        throw game_error("Cannot find base character");
    }

    void equip_vulture_sam_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_player_death>(origin_card, [=](player_ptr target, bool tried_save) {
            if (origin == target) {
                origin->set_character(find_base_character(origin->get_character()));
                origin->set_hp(4);
            }
        });
    }
}