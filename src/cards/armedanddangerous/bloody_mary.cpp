#include "bloody_mary.h"

#include "game/game.h"

namespace banggame {
    
    void effect_bloody_mary::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, effect_flags flags) {
            if (origin == p && bool(flags & effect_flags::is_bang)) {
                p->m_game->flash_card(target_card);
                p->draw_card(1, target_card);
            }
        });
    }
}