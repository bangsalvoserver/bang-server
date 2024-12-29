#include "josh_mccloud.h"

#include "effects/base/requests.h"

#include "cards/game_enums.h"

#include "game/possible_to_play.h"

namespace banggame {

    struct request_force_play_card : request_base {
        request_force_play_card(card_ptr origin_card, player_ptr target, card_ptr target_card)
            : request_base(origin_card, nullptr, target)
            , target_card(target_card) {}
        
        card_ptr target_card;

        void on_update() override {
            if (target_card == get_single_element(get_all_playable_cards(target, true))) {
                if (!target_card->modifier_response
                    && rn::all_of(target_card->responses, [](const effect_holder &holder) { return holder.target == TARGET_TYPE(none); })
                ) {
                    target->m_game->add_log("LOG_PLAYED_CARD", target_card, target);
                    target_card->move_to(pocket_type::shop_discard);
                    
                    for (const effect_holder &effect : target_card->responses) {
                        play_dispatch::play(target, target_card, effect, {}, utils::tag<"none">{});
                    }
                }
            } else {
                target->m_game->pop_request();
                target_card->add_short_pause();
                target_card->move_to(pocket_type::shop_discard);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_FORCE_PLAY_CARD", target_card};
            } else {
                return {"STATUS_FORCE_PLAY_CARD_OTHER", target, target_card};
            }
        }
    };

    bool effect_forced_play::can_play(card_ptr origin_card, player_ptr target) {
        if (auto req = target->m_game->top_request<request_force_play_card>(target_is{target})) {
            return origin_card == req->target_card;
        }
        return false;
    }

    void effect_forced_play::on_play(card_ptr origin_card, player_ptr target) {
        target->m_game->pop_request();
    }

    struct request_force_equip_card : request_picking_player {
        request_force_equip_card(card_ptr origin_card, player_ptr target, card_ptr target_card)
            : request_picking_player(origin_card, nullptr, target)
            , target_card(target_card) {}
        
        card_ptr target_card;

        card_list get_highlights() const override {
            return {target_card};
        }

        void on_update() override {
            if (get_all_equip_targets(target, target_card).empty()) {
                target->m_game->pop_request();
                target_card->add_short_pause();
                target_card->move_to(pocket_type::shop_discard);
            } else if (target_card->self_equippable()) {
                on_pick(target);
            }
        }

        bool can_pick(const_player_ptr target_player) const override {
            return !get_equip_error(target, target_card, target_player, {});
        }

        prompt_string pick_prompt(player_ptr target_player) const override {
            return get_equip_prompt(target, target_card, target_player);
        }

        void on_pick(player_ptr target_player) {
            target->m_game->pop_request();
            if (target_player == target) {
                target->m_game->add_log("LOG_EQUIPPED_CARD", target_card, target);
            } else {
                target->m_game->add_log("LOG_EQUIPPED_CARD_TO", target_card, target, target_player);
            }
            target_player->equip_card(target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_FORCE_EQUIP_CARD", target_card};
            } else {
                return {"STATUS_FORCE_EQUIP_CARD_OTHER", target, target_card};
            }
        }
    };

    void effect_josh_mccloud::on_play(card_ptr origin_card, player_ptr target) {
        card_ptr target_card = target->m_game->draw_shop_card();
        if (target_card->is_black()) {
            target->m_game->queue_request<request_force_equip_card>(origin_card, target, target_card);
        } else {
            target->m_game->queue_request<request_force_play_card>(origin_card, target, target_card);
        }
    }
}