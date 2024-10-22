#include "tornado.h"

#include "effects/base/pick.h"
#include "cards/game_enums.h"

#include "game/game.h"
#include "game/possible_to_play.h"

namespace banggame {

    struct request_tornado : request_picking {
        request_tornado(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}
        
        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && !target->m_game->is_usage_disabled(target_card);
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else if (rn::none_of(target->m_hand, [&](const_card_ptr c) { return can_pick(c); })) {
                target->m_game->pop_request();
                target->reveal_hand();
                target->draw_card(2, origin_card);
            } else if (target->m_hand.size() == 1) {
                auto_pick();
            }
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
            target->draw_card(2, origin_card);
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_TORNADO", origin_card};
            } else {
                return {"STATUS_TORNADO_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_tornado::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->queue_request<request_tornado>(origin_card, origin, target, flags);
    }

    struct request_tornado2 : request_base {
        using request_base::request_base;

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags) || target->empty_hand()) {
                target->m_game->pop_request();
            } else if (target->m_hand.size() <= 2) {
                handler_tornado2_response{}.on_play(origin_card, target, target->m_hand);
            }
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_TORNADO2", origin_card};
            } else {
                return {"STATUS_TORNADO2_OTHER", target, origin_card};
            }
        }
    };
    
    void effect_tornado2::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->queue_request<request_tornado2>(origin_card, origin, target, flags);
    }

    bool effect_tornado2_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_tornado2>(origin) != nullptr;
    }

    void handler_tornado2_response::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        origin->m_game->pop_request();
        origin->m_game->queue_action([=]{
            for (card_ptr target_card : target_cards) {
                player_ptr target = origin->get_next_player();
                if (target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, target_card);
                    origin->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
                } else {
                    origin->m_game->add_log("LOG_GIFTED_CARD", origin, target, target_card);
                }
                target->steal_card(target_card);
            }
        });
    }
}