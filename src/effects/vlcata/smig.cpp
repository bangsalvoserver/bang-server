#include "smig.h"
#include "game/game_table.h"
#include "effects/base/requests.h"

namespace banggame {
    void effect_smig_destroy::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        player_ptr owner = target_card->owner;
        origin->m_game->queue_request<request_missed_or_destroy_card>(origin_card, origin, owner, target_card);
    }
}