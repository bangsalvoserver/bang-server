#include "helena.h"
#include "game/game_table.h"
#include "effects/base/requests.h"

namespace banggame {
    void effect_helena_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_helena_force_play>(origin_card, origin, target);
    }
}