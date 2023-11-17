#include "russianroulette.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void equip_russianroulette::on_enable(card *target_card, player *target) {
        auto queue_russianroulette_request = [=](player *target) {
            auto req = std::make_shared<request_bang>(target_card, nullptr, target, effect_flags{}, 0);
            req->bang_damage = 2;
            target->m_game->queue_request(std::move(req));
        };
        queue_russianroulette_request(target);
        target->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, card *missed_card, effect_flags flags) {
            if (target_card == origin_card) {
                queue_russianroulette_request(*std::next(player_iterator(target)));
            }
        });
        target->m_game->add_listener<event_type::on_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (target_card == origin_card) {
                target->m_game->remove_listeners(target_card);
            }
        });
    }
}