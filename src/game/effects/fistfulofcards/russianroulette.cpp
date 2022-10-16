#include "russianroulette.h"

#include "../../game.h"
#include "../base/bang.h"

namespace banggame {

    void effect_russianroulette::on_enable(card *target_card, player *target) {
        auto queue_russianroulette_request = [=](player *target) {
            auto req = std::make_shared<request_bang>(target_card, nullptr, target);
            req->bang_damage = 2;
            target->m_game->queue_request(std::move(req));
        };
        queue_russianroulette_request(target);
        target->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, bool is_bang) {
            if (target_card == origin_card) {
                queue_russianroulette_request(std::next(player_iterator(target)));
            }
        });
        target->m_game->add_listener<event_type::before_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (target_card == origin_card) {
                target->m_game->remove_listeners(target_card);
            }
        });
    }
}