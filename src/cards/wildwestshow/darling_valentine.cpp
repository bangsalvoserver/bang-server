#include "darling_valentine.h"

#include "cards/base/requests.h"

#include "game/game.h"

namespace banggame {

    struct request_discard_hand : request_discard, resolvable_request {
        request_discard_hand(card *origin_card, player *target)
            : request_discard(origin_card, nullptr, target, target->m_hand.size()) {}
        
        void on_update() override {
            if (target->m_game->m_options.quick_discard_all || target->m_hand.size() <= 1) {
                on_resolve();
            }
        }

        void on_resolve() override {
            target->m_game->invoke_action([origin_card=origin_card, target=target]{
                target->m_game->pop_request();
                while (!target->empty_hand()) {
                    card *target_card = target->m_hand.front();
                    target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
                    target->discard_card(target_card);
                }
            });
        }
    };

    void equip_darling_valentine::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -1}, [=](player *origin) {
            int ncards = origin->m_hand.size();
            origin->m_game->queue_request<request_discard_hand>(target_card, origin);
            origin->m_game->queue_action([=]{
                origin->draw_card(ncards, target_card);
            }, 1);
        });
    }
}