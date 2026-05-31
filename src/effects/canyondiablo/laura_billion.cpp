#include "laura_billion.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/draw_check.h"
#include "effects/base/resolve.h"

#include "game/game_table.h"

namespace banggame {

    struct request_laura_billion : request_dismissable {
        request_laura_billion(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card)
            : request_dismissable(origin_card, origin, target, {}, 0)
            , target_card{target_card} {}
        
        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return { target_card };
        }

        void on_update() override {
            if (!target->alive() || target_card->pocket == pocket_type::player_hand) {
                pop_request();
            } else {
                auto_resolve();
            }
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_LAURA_BILLION", origin_card, target_card};
            } else {
                return {"STATUS_LAURA_BILLION_OTHER", target, origin_card, target_card};
            }
        }
    };

    static card_ptr get_laura_billion(player_ptr target) {
        return target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::draw_check_select });
    }
    
    void equip_laura_billion::on_enable(card_ptr target_card, player_ptr player_end) {
        player_end->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type) -> card_ptr {
            if (type == card_taker_type::draw_check_select && e_target == player_end) {
                return target_card;
            }
            return nullptr;
        });
        player_end->m_game->add_listener<event_type::on_draw_check_resolve>(target_card, [=](card_ptr origin_card, player_ptr player_begin, card_ptr e_target_card, card_ptr drawn_card) {
            int priority = 30;
            for (player_ptr current : player_begin->m_game->range_all_players(player_begin)) {
                if (current == player_end) break;
                if (current->alive() && get_laura_billion(current)) --priority;
            }

            player_end->m_game->queue_action([=]{
                player_end->m_game->queue_request<request_laura_billion>(target_card, player_begin, player_end, e_target_card);
            }, priority);
        });
    }

    bool effect_laura_billion::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_laura_billion>(target_is{origin}) != nullptr;
    }

    void effect_laura_billion::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_laura_billion>();
        card_ptr target_card = req->target_card;
        req->pop_request();
        
        origin->m_game->add_log("LOG_DRAWN_CARD", origin, target_card);
        target_card->add_short_pause();
        origin->add_to_hand(target_card);
    }
}