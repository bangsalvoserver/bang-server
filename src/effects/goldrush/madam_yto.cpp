#include "madam_yto.h"

#include "game/game_table.h"

#include "effects/base/beer.h"

namespace banggame {

    void equip_madam_yto::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_play_beer>({target_card, 1}, [=](player_ptr target, bool is_sold) {
            target_card->flash_card();
            p->draw_card(1, target_card);
        });
    }
}