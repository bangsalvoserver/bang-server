#include "ironhorse.h"

#include "game/game.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "ruleset.h"

namespace banggame {

    void equip_ironhorse::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>(origin_card, [=](player *target, shared_effect_context ctx) {
            origin->m_game->queue_action([=]{
                for (player *p : range_all_players(target)) {
                    if (p != ctx->skipped_player) {
                        origin->m_game->queue_request<request_bang>(origin_card, nullptr, p, effect_flag::multi_target);
                    }
                }
            });
        });
    }
}