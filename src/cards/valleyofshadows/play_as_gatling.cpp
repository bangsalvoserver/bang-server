#include "play_as_gatling.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {
    
    struct request_card_as_gatling : request_bang {
        using request_bang::request_bang;
        
        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CARD_AS_GATLING", origin_card};
            } else {
                return {"STATUS_CARD_AS_GATLING_OTHER", target, origin_card};
            }
        }
    };

    void handler_play_as_gatling::on_play(card *origin_card, player *origin, card *chosen_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", chosen_card, origin);
        origin->discard_card(chosen_card);

        auto targets = range_other_players(origin);
        auto flags = std::ranges::distance(targets) == 1 ? effect_flags::single_target : effect_flags{};
        for (player &p : targets) {
            if (!p.immune_to(chosen_card, origin, flags)) {
                origin->m_game->queue_request<request_card_as_gatling>(chosen_card, origin, &p, flags);
            }
        }
    }
}