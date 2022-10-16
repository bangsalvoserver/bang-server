#include "madam_yto.h"

#include "../../game.h"

namespace banggame {

    void effect_madam_yto::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_play_beer>({target_card, 1}, [=](player *target) {
            p->m_game->flash_card(target_card);
            p->draw_card(1, target_card);
        });
    }
}