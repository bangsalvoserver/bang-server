#include "claus_the_saint.h"

#include "effects/base/draw.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    struct request_claus_the_saint : request_base, interface_target_set_players {
        request_claus_the_saint(card_ptr origin_card, player_ptr target, shared_request_draw &&req_draw)
            : request_base(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        player_set remaining_targets;

        shared_request_draw req_draw;

        void on_update() override {
            if (update_count == 0) {
                for (player_ptr p : target->m_game->m_players) {
                    if (p->alive() && p != target) {
                        remaining_targets.add(p);
                    }
                }
                int ncards = target->m_game->num_alive() + req_draw->num_cards_to_draw - 1;
                for (int i=0; i<ncards; ++i) {
                    req_draw->phase_one_drawn_card()->move_to(pocket_type::selection, target);
                }
            }
            
            if (!remaining_targets) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    req_draw->add_to_hand_phase_one(target->m_game->m_selection.front());
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_CLAUS_THE_SAINT", origin_card};
            } else {
                return {"STATUS_CLAUS_THE_SAINT_OTHER", target, origin_card};
            }
        }

        bool in_target_set(const_player_ptr target_player) const override {
            return remaining_targets.contains(target_player);
        }
    };
    
    void equip_claus_the_saint::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_draw_handlers>(target_card, [=](player_ptr origin, shared_request_draw req_draw) {
            if (origin == target) {
                req_draw->handlers.push_back(target_card);
            }
        });
        
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, card_ptr origin_card, shared_request_draw req_draw) {
            if (origin == target && origin_card == target_card) {
                target->m_game->queue_request<request_claus_the_saint>(target_card, target, std::move(req_draw));
            }
        });
    }

    bool effect_claus_the_saint_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_claus_the_saint>(target_is{origin}) != nullptr;
    }

    void effect_claus_the_saint_response::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        auto req = origin->m_game->top_request<request_claus_the_saint>();
        req->remaining_targets.remove(target);
    }
}