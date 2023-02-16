#include "josh_mccloud.h"

#include "game/game.h"
#include "game/filters.h"
#include "cards/base/requests.h"
#include "game/possible_to_play.h"

namespace banggame {

    struct request_force_play_card : request_base {
        request_force_play_card(card *origin_card, player *target, card *target_card)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond)
            , target_card(target_card) {}
        
        card *target_card;

        void on_update() override {
            auto_respond();
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_FORCE_PLAY_CARD", target_card};
            } else {
                return {"STATUS_FORCE_PLAY_CARD_OTHER", target, target_card};
            }
        }
    };

    bool effect_forced_play::can_respond(card *origin_card, player *target) {
        if (auto req = target->m_game->top_request<request_force_play_card>(target)) {
            return origin_card == req->target_card;
        }
        return false;
    }

    void effect_forced_play::on_play(card *origin_card, player *target) {
        target->m_game->pop_request();
    }

    struct request_force_equip_card : request_base {
        request_force_equip_card(card *origin_card, player *target, card *target_card)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond)
            , target_card(target_card) {}
        
        card *target_card;

        std::vector<card *> get_highlights() const override {
            return {target_card};
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_FORCE_EQUIP_CARD", target_card};
            } else {
                return {"STATUS_FORCE_EQUIP_CARD_OTHER", target, target_card};
            }
        }
    };

    game_string effect_forced_equip::verify(card *origin_card, player *origin, player *target) {
        if (auto req = target->m_game->top_request<request_force_equip_card>(origin)) {
            card *target_card = req->target_card;
            MAYBE_RETURN(check_player_filter(origin, target_card->equip_target, target));
            if (card *equipped = target->find_equipped_card(target_card)) {
                return {"ERROR_DUPLICATED_CARD", equipped};
            } else {
                return {};
            }
        }
        return "ERROR_INVALID_RESPONSE";
    }

    game_string effect_forced_equip::on_prompt(card *origin_card, player *origin, player *target) {
        card *target_card = origin->m_game->top_request<request_force_equip_card>()->target_card;
        for (const auto &e : target_card->equips) {
            if (auto prompt_message = e.on_prompt(target_card, origin, target)) {
                return prompt_message;
            }
        }
        return {};
    }
    
    void effect_forced_equip::on_play(card *origin_card, player *origin, player *target) {
        card *target_card = origin->m_game->top_request<request_force_equip_card>()->target_card;
        origin->m_game->invoke_action([&]{
            origin->m_game->pop_request();

            if (origin == target) {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP", target_card, origin);
            } else {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", target_card, origin, target);
            }
            target->equip_card(target_card);
        });
    }

    void effect_josh_mccloud::on_play(card *origin_card, player *target) {
        auto *card = target->m_game->draw_shop_card();
        auto discard_drawn_card = [&]{
            target->m_game->add_short_pause(card);
            target->m_game->move_card(card, pocket_type::shop_discard);
        };
        if (card->is_black()) {
            if (!target->m_game->check_flags(game_flags::disable_equipping)) {
                auto equip_set = make_equip_set(target, card) | ranges::to<std::vector>;
                if (equip_set.empty()) {
                    discard_drawn_card();
                } else if (equip_set.size() == 1) {
                    equip_set.front()->equip_card(card);
                } else {
                    target->m_game->queue_request<request_force_equip_card>(origin_card, target, card);
                }
            } else {
                discard_drawn_card();
            }
        } else if (is_possible_to_play(target, card)) {
            target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
        } else {
            discard_drawn_card();
        }
    }
}