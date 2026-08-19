#include "el_bagito.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_el_bagito::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card,
            [target, target_card](player_ptr origin, card_ptr played_card) {
                if (origin != target && target->alive()) {
                    if (played_card->name == "STAGECOACH") {
                        target_card->flash_card();
                        target->draw_card(2, target_card);
                    } else if (played_card->name == "WELLS_FARGO" || played_card->name == "PONY_EXPRESS") {
                        target_card->flash_card();
                        target->draw_card(3, target_card);
                    }
                }
            });
    }
}