#include "leland.h"

#include "game/game.h"

#include "effects/base/generalstore.h"

#include "ruleset.h"

namespace banggame {

    void equip_leland::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>(origin_card, [=](player_ptr target, shared_effect_context ctx) {
            origin->m_game->queue_action([=]{
                player_list targets = range_all_players(target) | rn::to_vector;
                for (player_ptr p : targets) {
                    if (p != ctx->skipped_player) {
                        origin->m_game->top_of_deck()->move_to(pocket_type::selection);
                    }
                }
                for (player_ptr p : targets) {
                    if (p != ctx->skipped_player) {
                        origin->m_game->queue_request<request_generalstore>(origin_card, nullptr, p);
                    }
                }
            });
        });
    }
}