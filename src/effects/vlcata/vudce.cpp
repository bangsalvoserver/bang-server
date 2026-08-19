#include "vudce.h"
#include "game/game_table.h"
#include "effects/base/requests.h"

namespace banggame {
    void effect_vudce_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_vudce_choose>(origin_card, origin, target);
    }
}