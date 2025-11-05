#include "tornado.h"

#include "effects/base/pick.h"
#include "effects/base/escapable.h"

#include "cards/game_enums.h"

#include "effects/base/pick.h"
#include "effects/base/gift_card.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"
#include "game/prompts.h"

#include "poker.h"

namespace banggame {

    struct request_tornado : request_picking, escapable_request {
        request_tornado(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_picking(origin_card, origin, target, flags) {}
        
        prompt_string escape_prompt(player_ptr owner) const override {
            if (owner->is_bot()) {
                return "BOT_ESCAPE_TORNADO";
            }
            return {};
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            return prompts::bot_check_discard_card(target, target_card);
        }
        
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

        card_list get_highlights(player_ptr owner) const override {
            card_list result;
            target->m_game->call_event(event_type::get_selected_cards{ origin_card, owner, result });
            return result;
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags) || target->empty_hand()) {
                target->m_game->pop_request();
            } else if (target->m_hand.size() <= 2) {
                on_resolve(target->m_hand);
            }
        }

        void on_resolve(const card_list &target_cards) {
            target->m_game->pop_request();

            target->m_game->add_listener<event_type::get_selected_cards>(origin_card,
                [origin_card=origin_card, target=target, target_cards](card_ptr e_origin_card, player_ptr owner, card_list &result) {
                    if (origin_card == e_origin_card) {
                        if (owner == target) {
                            for (card_ptr c : target_cards) {
                                result.push_back(c);
                            }
                        } else {
                            for (card_ptr c : target->m_hand) {
                                result.push_back(c);
                            }
                        }
                    }
                });
            
            target->m_game->queue_action([origin_card=origin_card, target=target, target_cards]{
                target->m_game->remove_listeners(origin_card);

                player_ptr target_player = target->get_next_player();
                for (card_ptr target_card : target_cards) {
                    effect_gift_card{true}.on_play(origin_card, target, target_card, target_player);
                }
            });
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
        return origin->m_game->top_request<request_tornado2>(target_is{origin}) != nullptr;
    }

    prompt_string effect_tornado2_response::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        for (card_ptr target_card : ctx.selected_cards) {
            MAYBE_RETURN(prompts::bot_check_discard_card(origin, target_card));
        }
        return {};
    }

    void effect_tornado2_response::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        auto req = origin->m_game->top_request<request_tornado2>();
        req->on_resolve(ctx.selected_cards);
    }
}