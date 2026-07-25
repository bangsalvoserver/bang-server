#include "josiah_tung.h"

#include "effects/base/can_play_card.h"
#include "effects/base/steal_destroy.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "utils/random_element.h"

namespace banggame {

    void equip_josiah_tung::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            if (origin == target) {
                origin->m_game->queue_action([=]{
                    origin->m_game->queue_request<request_can_play_card>(target_card, nullptr, origin);
                }, -26);
            }
        });
    }

    static auto get_discardable_cards(card_ptr origin_card, player_ptr origin) {
        return origin->m_hand | rv::filter([=](card_ptr target_card) {
            return !effect_discard{}.get_error(origin_card, origin, target_card);
        });
    }

    bool effect_josiah_tung::can_play(card_ptr origin_card, player_ptr origin) {
        return !rn::empty(get_discardable_cards(origin_card, origin));
    }

    void effect_josiah_tung::on_play(card_ptr origin_card, player_ptr origin) {
        card_ptr target_card = random_element(get_discardable_cards(origin_card, origin), origin->m_game->rng);
        effect_discard{}.on_play(origin_card, origin, target_card);
        origin->draw_card(2, origin_card);
    }
}