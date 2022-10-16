#include "bloody_mary.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_bloody_mary::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, bool is_bang) {
            if (origin == p && is_bang) {
                p->m_game->flash_card(target_card);
                p->draw_card(1, target_card);
            }
        });
    }
}