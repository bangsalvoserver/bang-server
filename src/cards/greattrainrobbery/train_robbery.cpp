#include "train_robbery.h"

#include "game/game.h"

#include "cards/base/bang.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_train_robbery_choose : request_bang {
        request_train_robbery_choose(card *origin_card, player *origin, player *target, card *target_card)
            : request_bang(origin_card, origin, target)
            , target_card(target_card) {}
        
        card *target_card;

        void on_update() override {
            if (target->empty_hand()) {
                auto_respond();
            }
        }

        bool can_pick(card *c) const override {
            return c == target_card && num_cards_used() == 0;
        }

        void on_pick(card *) override {
            origin->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        game_string status_text(player *owner) const override {
            if (num_cards_used() == 0) {
                if (owner == target) {
                    return {"STATUS_TRAIN_ROBBERY_CHOOSE", origin_card, target_card};
                } else {
                    return {"STATUS_TRAIN_ROBBERY_CHOOSE_OTHER", target, origin_card, target_card};
                }
            } else {
                return request_bang::status_text(owner);
            }
        }
    };

    struct request_train_robbery : request_base {
        using request_base::request_base;

        std::vector<card *> selected_cards;

        bool is_valid() const {
            return std::ranges::any_of(target->m_table, [&](card *target_card) {
                return can_pick(target_card);
            });
        }

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags) || !is_valid()) {
                target->m_game->pop_request();
            } else {
                auto_pick();
            }
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target
                && !target_card->is_black() && !ranges::contains(selected_cards, target_card);
        }

        void on_pick(card *target_card) override {
            selected_cards.push_back(target_card);
            
            target->m_game->queue_action([req=target->m_game->top_request<request_train_robbery>()] () mutable {
                if (req->target->alive() && req->is_valid()) {
                    req->flags &= ~effect_flags::escapable;
                    req->state = request_state::pending;
                    req->target->m_game->queue_request_front(std::move(req));
                }
            }, 1);
            target->m_game->pop_request();
            target->m_game->queue_request_front<request_train_robbery_choose>(origin_card, origin, target, target_card);
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_TRAIN_ROBBERY", origin_card};
            } else {
                return {"STATUS_TRAIN_ROBBERY_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_train_robbery::on_prompt(card *origin_card, player *origin, player *target) {
        if (std::ranges::all_of(target->m_table, &card::is_black)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_train_robbery::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_train_robbery>(origin_card, origin, target, flags);
    }
}