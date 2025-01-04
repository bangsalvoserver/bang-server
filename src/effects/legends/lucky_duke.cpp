#include "lucky_duke.h"

#include "effects/base/vulture_sam.h"
#include "effects/base/draw_check.h"
#include "effects/base/pick.h"

#include "game/game.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct request_lucky_duke_legend : selection_picker {
        request_lucky_duke_legend(card_ptr origin_card, player_ptr origin, player_ptr target, shared_request_check &&req)
            : selection_picker(origin_card, origin, target, {}, 115)
            , req{std::move(req)} {}
        
        shared_request_check req;

        card_list get_highlights() const override {
            return { req->origin_card };
        }

        void on_update() override {
            if (!live) {
                for (int i = 0; i < 2; ++i) {
                    card_ptr target_card = target->m_game->top_of_deck();
                    target->m_game->add_log("LOG_REVEALED_CARD", target, target_card);
                    target_card->move_to(pocket_type::selection);
                }
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            req->on_pick(target_card);
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            if (target->is_bot()) {
                if (bot_suggestion::is_target_friend(target, origin) != req->is_lucky(target_card)) {
                    return "PROMPT_BAD_DRAW";
                }
            }
            return {};
        }
        
        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", req->origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, req->origin_card};
            }
        }
    };

    static card_ptr get_lucky_duke(player_ptr target) {
        card_ptr origin_card = nullptr;
        target->m_game->call_event(event_type::check_card_taker{ target, card_taker_type::draw_check_start, origin_card });
        return origin_card;
    }
    
    void equip_lucky_duke_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_card_taker>(target_card, [=](player_ptr e_target, card_taker_type type, card* &value) {
            if (type == card_taker_type::draw_check_start && e_target == target) {
                value = target_card;
            }
        });

        target->m_game->add_listener<event_type::on_draw_check_start>(target_card, [=](player_ptr origin, shared_request_check req, bool &handled) {
            if (!handled && req->origin_card && req->origin_card->deck == card_deck_type::main_deck) {
                if (rn::none_of(origin->m_game->range_all_players(origin)
                    | rv::take_while([=](const_player_ptr current) { return current != target; })
                    | rv::filter(&player::alive),
                    get_lucky_duke)
                ) {
                    handled = true;
                    target->m_game->queue_request<request_lucky_duke_legend>(target_card, origin, target, std::move(req));
                }
            }
        });

        target->m_game->add_listener<event_type::on_draw_check_resolve>(target_card, [=](card_ptr origin_card, player_ptr origin, card_ptr target_card, card_ptr drawn_card) {
            if (target == target->m_game->m_playing && target_card == drawn_card && origin_card && origin_card->deck == card_deck_type::main_deck) {
                target->m_game->add_log("LOG_DRAWN_CARD", target, target_card);
                target->add_to_hand(target_card);
            }
        });
    }
}