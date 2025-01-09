#include "russianroulette.h"

#include "game/game_table.h"
#include "effects/base/bang.h"
#include "effects/base/damage.h"

namespace banggame {

    void equip_russianroulette::on_enable(card_ptr target_card, player_ptr target) {
        auto queue_russianroulette_request = [=](player_ptr target) {
            auto req = std::make_shared<request_bang>(target_card, nullptr, target, effect_flags{}, 0);
            req->bang_damage = 2;
            target->m_game->queue_request(std::move(req));
        };
        queue_russianroulette_request(target);
        target->m_game->add_listener<event_type::on_missed>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr missed_card, effect_flags flags) {
            if (target_card == origin_card) {
                queue_russianroulette_request(target->get_next_player());
            }
        });
        target->m_game->add_listener<event_type::on_hit>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (target_card == origin_card) {
                target->m_game->remove_listeners(target_card);
            }
        });
    }
}