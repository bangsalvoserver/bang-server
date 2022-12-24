#include "play_as_gatling.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void handler_play_as_gatling::on_play(card *origin_card, player *origin, card *chosen_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", chosen_card, origin);
        origin->discard_card(chosen_card);

        auto targets = ranges::to<std::vector>(range_other_players(origin));
        auto flags = effect_flags::play_as_bang | effect_flags::multi_target;
        if (targets.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (player *p : targets) {
            if (!p->immune_to(chosen_card, origin, flags)) {
                origin->m_game->queue_request<request_bang>(chosen_card, origin, p, flags);
            }
        }
    }
}