#include "annie_oakey.h"

#include "game/game_table.h"

#include "effects/base/draw.h"
#include "effects/base/resolve.h"

namespace banggame {

    struct request_annie_oakey : request_base, interface_resolvable {
        request_annie_oakey(card_ptr origin_card, player_ptr target, shared_request_draw &&req_draw)
            : request_base(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        shared_request_draw req_draw;

        void on_update() override {
            if (update_count == 0) {
                req_draw->cleanup_selection();
            }
            if (!target->alive() || req_draw->num_drawn_cards >= req_draw->num_cards_to_draw) {
                target->m_game->pop_request();
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            req_draw->add_to_hand_phase_one(req_draw->phase_one_drawn_card());
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_ANNIE_OAKEY", origin_card};
            } else {
                return {"STATUS_ANNIE_OAKEY_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_annie_oakey::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_draw_handlers>(target_card, [=](player_ptr origin, shared_request_draw req_draw) {
            if (origin == target) {
                req_draw->handlers.push_back(target_card);
            }
        });
        
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, card_ptr origin_card, shared_request_draw req_draw) {
            if (origin == target && origin_card == target_card) {
                target->m_game->queue_request<request_annie_oakey>(target_card, target, std::move(req_draw));
            }
        });
    }

    bool effect_annie_oakey::can_play(card_ptr origin_card, player_ptr target) {
        return target->m_game->top_request<request_annie_oakey>(target_is{target}) != nullptr;
    }

    void effect_annie_oakey::on_play(card_ptr origin_card, player_ptr target) {
        shared_request_draw req_draw = target->m_game->top_request<request_annie_oakey>()->req_draw;

        card_ptr drawn_card = req_draw->phase_one_drawn_card();
        drawn_card->set_visibility(card_visibility::shown);
        drawn_card->add_short_pause();
        req_draw->add_to_hand_phase_one(drawn_card);

        if (std::invoke(fn, drawn_card->sign)) {
            target->draw_card(ncards, origin_card);
        }
    }
}