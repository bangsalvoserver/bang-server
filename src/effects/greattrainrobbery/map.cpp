#include "map.h"

#include "game/game.h"
#include "effects/base/pick.h"
#include "effects/base/resolve.h"
#include "cards/game_enums.h"

namespace banggame {

    struct request_map : selection_picker, interface_resolvable {
        request_map(card_ptr origin_card, player_ptr target)
            : selection_picker(origin_card, nullptr, target) {}
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<2; ++i) {
                    target->m_game->top_of_deck()->move_to(pocket_type::selection, target);
                }
            }
        }

        bool move_card_to_deck() const {
            return target->m_game->check_flags(game_flag::phase_one_override)
                || target->m_game->check_flags(game_flag::phase_one_draw_discard) && !target->m_game->m_discards.empty();
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
            if (move_card_to_deck()) {
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->m_selection.front()->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden);
                }
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (move_card_to_deck()) {
                target_card->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden);
            }
            while (auto not_target = target->m_game->m_selection | rv::filter([&](card_ptr selection_card) {
                return selection_card != target_card;
            })) {
                card_ptr discarded = *not_target.begin();
                target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, discarded);
                discarded->move_to(pocket_type::discard_pile);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_MAP", origin_card};
            } else {
                return {"STATUS_MAP_OTHER", target, origin_card};
            }
        }
    };

    void equip_map::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_turn_start>({origin_card, -5}, [=](player_ptr target) {
            if (origin == target) {
                origin->m_game->queue_request<request_map>(origin_card, origin);
            }
        });
    }
}