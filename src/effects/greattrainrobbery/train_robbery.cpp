#include "train_robbery.h"

#include "game/game.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

namespace banggame {

    struct request_train_robbery : request_base, interface_target_set_cards {
        using request_base::request_base;

        std::vector<card *> selected_cards;

        void on_update() override {
            if (!target->alive() || target->immune_to(origin_card, origin, flags)
                || rn::none_of(target->m_table, [&](card *target_card) {
                    return in_target_set(target_card);
                })
            ) {
                target->m_game->pop_request();
            }
        }

        bool in_target_set(const card *target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target
                && !target_card->is_black() && !rn::contains(selected_cards, target_card);
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

    game_string effect_train_robbery_response::get_error(card *origin_card, player *origin, card *target_card) {
        if (auto req = origin->m_game->top_request<request_train_robbery>(origin)) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    void effect_train_robbery_response::on_play(card *origin_card, player *origin, card *target_card) {
        auto req = origin->m_game->top_request<request_train_robbery>();
        req->selected_cards.push_back(target_card);

        req->flags.remove(effect_flag::escapable);
        req->flags.remove(effect_flag::single_target);
    }

    void effect_train_robbery_discard::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<request_train_robbery>();
        card *target_card = req->selected_cards.back();

        origin->m_game->add_log("LOG_DISCARDED_SELF_CARD", origin, target_card);
        origin->discard_card(target_card);
    }

    struct request_train_robbery_bang : request_bang {
        request_train_robbery_bang(card *origin_card, player *origin, player *target, card *target_card)
            : request_bang(origin_card, origin, target, {}, 21)
            , target_card{target_card} {}
        
        card *target_card;

        std::vector<card *> get_highlights() const override {
            return { target_card };
        }
    };

    void effect_train_robbery_bang::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<request_train_robbery>();
        card *target_card = req->selected_cards.back();

        origin->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", origin, target_card, 1);
        origin->m_game->queue_request<request_train_robbery_bang>(req->origin_card, req->origin, origin, target_card);
    }
}