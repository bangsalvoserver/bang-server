#include "bloody_mary.h"

#include "cards/game_enums.h"
#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_bloody_mary::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr missed_card, effect_flags flags) {
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