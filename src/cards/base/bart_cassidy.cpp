#include "bart_cassidy.h"

#include "game/game.h"
#include "cards/base/damage.h"

namespace banggame {
    
    void equip_bart_cassidy::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 1}, [p, target_card](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->flash_card(target_card);
                        target->draw_card(damage, target_card);
                    }
                });
            }
        });
    }
}