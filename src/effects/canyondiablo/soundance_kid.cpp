#include "soundance_kid.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"

#include "game/game_table.h"

namespace banggame {

    void equip_soundance_kid::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 5}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin == p && flags.check(effect_flag::is_bang)) {
                target_card->flash_card();
                origin->draw_card(1, target_card);
            }
        });
    }
}