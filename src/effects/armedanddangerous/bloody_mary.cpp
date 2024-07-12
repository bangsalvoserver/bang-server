#include "bloody_mary.h"

#include "cards/game_enums.h"
#include "effects/base/bang.h"

#include "game/game.h"

namespace banggame {
    
    void equip_bloody_mary::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, card *missed_card, effect_flags flags) {
            if (origin == p && flags.check(effect_flag::is_bang)) {
                p->m_game->queue_action([=]{
                    if (p->alive()) {
                        target_card->flash_card();
                        p->draw_card(1, target_card);
                    }
                });
            }
        });
    }
}