#include "budha.h"
#include "cards/game_events.h"
#include "game/game_table.h"
#include "effects/base/requests.h"

namespace banggame {
    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card,
            [target, target_card](player_ptr origin, card_ptr played_card) {
                if (origin == target && (played_card->name == "BEER" || played_card->name == "SALOON" || played_card->name == "WHISKY")) {
                    target_card->flash_card();
                    target->m_game->queue_action([target, target_card]{
                        for (int i = 0; i < 3 && !target->m_game->m_deck.empty(); ++i) {
                            card_ptr drawn = target->m_game->top_of_deck();
                            target->m_game->move_card(drawn, pocket_type::selection, target);
                        }
                    });
                }
            });
    }
}