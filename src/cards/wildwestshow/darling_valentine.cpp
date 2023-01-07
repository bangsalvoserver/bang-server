#include "darling_valentine.h"

#include "cards/base/requests.h"

#include "game/game.h"

namespace banggame {

    struct request_discard_hand : request_base, resolvable_request {
        request_discard_hand(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target)
            , ncards(int(target->m_hand.size())) {}
        
        int ncards;
        
        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
                target->discard_card(target_card);
            });
        }
        
        void on_update() override {
            if (target->m_game->m_options.quick_discard_all || target->m_hand.size() <= 1) {
                on_resolve();
            }
        }

        void on_resolve() override {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                while (!target->empty_hand()) {
                    on_pick(target->m_hand.front());
                }
                if (ncards > 0) {
                    target->draw_card(ncards, origin_card);
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_DISCARD_HAND", origin_card};
            } else {
                return {"STATUS_DISCARD_HAND_OTHER", origin_card, target};
            }
        }
    };

    void equip_darling_valentine::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -1}, [=](player *origin) {
            origin->m_game->queue_request<request_discard_hand>(target_card, origin);
        });
    }
}