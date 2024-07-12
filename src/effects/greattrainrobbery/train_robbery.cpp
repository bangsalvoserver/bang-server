#include "train_robbery.h"

#include "game/game.h"

#include "effects/base/bang.h"
#include "effects/base/pick.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_train_robbery_choose : request_bang, interface_picking {
        request_train_robbery_choose(card *origin_card, player *origin, player *target, card *target_card)
            : request_bang(origin_card, origin, target, {}, 21)
            , target_card(target_card) {}
        
        card *target_card;

        void on_update() override {
            if (target->empty_hand()) {
                auto_resolve();
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

        std::vector<card *> get_highlights() const override {
            return {target_card};
        }

        game_string status_text(player *owner) const override {
            if (num_cards_used() == 0) {
                if (owner == target) {
                    return {"STATUS_TRAIN_ROBBERY_CHOOSE", origin_card, target_card};
                } else {
                    return {"STATUS_TRAIN_ROBBERY_CHOOSE_OTH", target, origin_card, target_card};
                }
            } else {
                return request_bang::status_text(owner);
            }
        }
    };

    struct request_train_robbery : request_picking {
        using request_picking::request_picking;

        std::vector<card *> selected_cards;

        void on_update() override {
            if (!target->alive() || target->immune_to(origin_card, origin, flags)
                || rn::none_of(target->m_table, [&](card *target_card) {
                    return can_pick(target_card);
                })
            ) {
                target->m_game->pop_request();
            } else {
                auto_pick();
            }
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target
                && !target_card->is_black() && !rn::contains(selected_cards, target_card);
        }

        void on_pick(card *target_card) override {
            selected_cards.push_back(target_card);
            flags.remove(effect_flag::escapable);
            flags.remove(effect_flag::single_target);
            
            target->m_game->queue_request<request_train_robbery_choose>(origin_card, origin, target, target_card);
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
        if (rn::all_of(target->m_table, &card::is_black)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_train_robbery::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_train_robbery>(origin_card, origin, target, flags, 20);
    }
}