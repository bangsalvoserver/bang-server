#include "bloody_mary.h"

#include "cards/game_enums.h"
#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_bloody_mary::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
            if (req->origin == origin && flags.check(effect_flag::is_bang)) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {
                        target_card->flash_card();
                        origin->draw_card(1, target_card);
                    }
                });
            }
        });
    }
}