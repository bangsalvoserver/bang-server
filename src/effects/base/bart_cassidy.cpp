#include "bart_cassidy.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "effects/base/damage.h"

namespace banggame {

    game_string equip_bart_cassidy::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return prompts::prompt_target_ghost(origin_card, origin, target);
    }
    
    void equip_bart_cassidy::on_enable(card_ptr target_card, player_ptr p) const {
        p->m_game->add_listener<event_type::on_hit>({target_card, 1}, [p, target_card, ncards=ncards](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target_card->flash_card();
                        target->draw_card(damage * ncards, target_card);
                    }
                });
            }
        });
    }
}
