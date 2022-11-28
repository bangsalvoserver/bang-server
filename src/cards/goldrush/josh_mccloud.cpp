#include "josh_mccloud.h"

#include "game/game.h"
#include "cards/base/requests.h"

namespace banggame {

    struct request_force_play_card : request_base {
        request_force_play_card(card *origin_card, player *target, card *target_card)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond)
            , target_card(target_card) {}
        
        card *target_card;

        bool auto_resolve() override {
            return auto_respond();
        }

        bool can_respond(player *p, card *c) const override {
            return p == target && c == target_card;
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
        if (auto *req = target->m_game->top_request_if<request_force_play_card>(target)) {
            if (origin_card == req->target_card) {
                return true;
            } else if (req->target_card->modifier == card_modifier_type::shopchoice) {
                return origin_card->pocket == pocket_type::hidden_deck
                    && origin_card->get_tag_value(tag_type::shopchoice) == req->target_card->get_tag_value(tag_type::shopchoice);
            }
        }
        return false;
    }

    void effect_forced_play::on_play(card *origin_card, player *target) {
        target->m_game->pop_request();
    }

    void effect_josh_mccloud::on_play(card *origin_card, player *target) {
        auto *card = target->m_game->draw_shop_card();
        auto discard_drawn_card = [&]{
            target->m_game->move_card(card, pocket_type::shop_discard, nullptr, show_card_flags::pause_before_move);
        };
        if (card->color == card_color_type::black) {
            if (!target->m_game->check_flags(game_flags::disable_equipping)) {
                auto equip_set = target->make_equip_set(card);
                if (equip_set.empty()) {
                    discard_drawn_card();
                } else if (equip_set.size() == 1) {
                    equip_set.front()->equip_card(card);
                } else {
                    target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
                }
            } else {
                discard_drawn_card();
            }
        } else if (card->has_tag(tag_type::shopchoice) || target->is_possible_to_play(card)) {
            target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
        } else {
            discard_drawn_card();
        }
    }
}