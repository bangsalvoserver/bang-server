#include "leland.h"

#include "game/game_table.h"

#include "effects/base/generalstore.h"

#include "ruleset.h"

namespace banggame {

    void equip_leland::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>(origin_card, [=](player_ptr target, shared_locomotive_context ctx) {
            origin->m_game->queue_action([=]{
                for (player_ptr p : target->m_game->range_alive_players(target)) {
                    if (p != ctx->skipped_player) {
                        origin->m_game->top_of_deck()->move_to(pocket_type::selection);
                        origin->m_game->queue_request<request_generalstore>(origin_card, nullptr, p);
                    }
                }
            });
        });
    }
}