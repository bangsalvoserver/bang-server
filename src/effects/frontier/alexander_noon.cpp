#include "alexander_noon.h"

#include "effects/base/draw_check.h"
#include "effects/base/resolve.h"

#include "game/game_table.h"

namespace banggame {

    struct request_alexander_noon : request_dismissable {
        request_alexander_noon(card_ptr origin_card, player_ptr target, shared_request_check &&req)
            : request_dismissable(origin_card, nullptr, target, {}, 120)
            , req{std::move(req)} {}
        
        shared_request_check req;

        card_list get_highlights(player_ptr owner) const override {
            return { req->origin_card };
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_ALEXANDER_NOON", origin_card, req->origin_card};
            } else {
                return {"STATUS_ALEXANDER_NOON_OTHER", target, origin_card, req->origin_card};
            }
        }
    };

    void equip_alexander_noon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_check_start>({ target_card, -1 }, [=](player_ptr origin, shared_request_check req) {
            if (req->target == target && !req->drawn_card && !req->handlers.contains(target_card)) {
                req->handlers.add(target_card);
                target->m_game->queue_request<request_alexander_noon>(target_card, target, std::move(req));
                return true;
            }
            return false;
        });
    }
    
    struct request_alexander_noon_draw : request_dismissable, interface_picking {
        request_alexander_noon_draw(card_ptr origin_card, player_ptr target, shared_request_check &&req)
            : request_dismissable(origin_card, nullptr, target, {}, 115)
            , req{std::move(req)} {}
        
        shared_request_check req;

        card_list get_highlights(player_ptr owner) const override {
            return { req->origin_card };
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            if (target->is_bot()) {
                auto result = req->get_result(target_card);
                if (result.indifferent || !result.lucky) {
                    return "BOT_BAD_DRAW";
                }
            }
            return {};
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->discard_used_card(target_card);
            req->on_pick(target_card);
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", req->origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, req->origin_card};
            }
        }
    };

    bool effect_alexander_noon::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_alexander_noon>(target_is{origin}) != nullptr;
    }

    void effect_alexander_noon::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_alexander_noon>();
        req->pop_request();
        origin->m_game->queue_request<request_alexander_noon_draw>(origin_card, origin, std::move(req->req));
    }
}