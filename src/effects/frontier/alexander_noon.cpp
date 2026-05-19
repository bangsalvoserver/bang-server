#include "alexander_noon.h"

#include "effects/base/draw_check.h"
#include "effects/base/can_play_card.h"

#include "game/game_table.h"

namespace banggame {

    void equip_alexander_noon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_check_start>({ target_card, -1 }, [=](player_ptr origin, shared_request_check req) {
            if (req->target == target && !req->drawn_card && !req->handlers.contains(target_card)) {
                req->handlers.add(target_card);
                target->m_game->queue_request<request_can_play_card>(target_card, nullptr, target, effect_flags{}, 120);
                return true;
            }
            return false;
        });
    }
    
    struct request_alexander_noon : request_picking, interface_resolvable {
        request_alexander_noon(card_ptr origin_card, player_ptr target, shared_request_check &&req)
            : request_picking(origin_card, nullptr, target, {}, 115)
            , req{std::move(req)} {}
        
        shared_request_check req;

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->discard_card(target_card);
            req->on_pick(target_card);
        }
        
        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            pop_request();
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", req->origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, req->origin_card};
            }
        }
    };

    void effect_alexander_noon::on_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_check_base>(target_is{origin})) {
            origin->m_game->queue_request<request_alexander_noon>(origin_card, origin, std::move(req));
        } else {
            throw game_error("invalid request access in effect_alexander_noon: top_request is not request_check_base");
        }
    }
}