#include "clash_the_stampede.h"

#include "cards/wildwestshow/youl_grinner.h"

#include "game/game.h"

namespace banggame {

    void equip_clash_the_stampede::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            if (target == origin) {
                auto max_ncards = rn::max(target->m_game->m_players
                    | rv::transform([](player *p) { return p->m_hand.size(); }));
                for (player *p : range_other_players(target)) {
                    if (p->m_hand.size() == max_ncards) {
                        target->m_game->queue_request<request_youl_grinner>(target_card, target, p);
                    }
                }
            }
        });
    }
}