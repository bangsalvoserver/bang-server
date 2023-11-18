#include "map.h"

#include "game/game.h"

namespace banggame {

    struct request_map : selection_picker, resolvable_request {
        request_map(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<2; ++i) {
                    target->m_game->move_card(target->m_game->top_of_deck(), pocket_type::selection, target);
                }
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::main_deck, nullptr, card_visibility::hidden);
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->m_game->move_card(target_card, pocket_type::discard_pile);
            on_resolve();
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_MAP", origin_card};
            } else {
                return {"STATUS_MAP_OTHER", target, origin_card};
            }
        }
    };

    void equip_map::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_turn_start>({origin_card, 2}, [=](player *target) {
            if (origin == target) {
                origin->m_game->queue_request<request_map>(origin_card, origin);
            }
        });
    }
}