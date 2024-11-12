#include "clash_the_stampede.h"

#include "effects/wildwestshow/youl_grinner.h"

#include "game/game.h"
#include "game/possible_to_play.h"

namespace banggame {

    void equip_clash_the_stampede::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({origin_card, -2}, [=](player_ptr origin) {
            if (target == origin) {
                size_t max_num_cards = rn::max(target->m_game->m_players | rv::transform([](player_ptr p) {
                    return p->m_hand.size();
                }));
                if (target->m_hand.size() < max_num_cards) {
                    for (player_ptr p : target->m_game->range_other_players(target)) {
                        if (p->m_hand.size() == max_num_cards) {
                            target->m_game->queue_request<request_youl_grinner>(origin_card, target, p);
                        }
                    }
                }
            }
        });
    }

}