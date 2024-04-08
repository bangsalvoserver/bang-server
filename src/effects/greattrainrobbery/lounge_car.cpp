#include "lounge_car.h"

#include "game/game.h"

#include "game/filters.h"

namespace banggame {

    struct request_lounge_car : request_base, interface_target_set {
        request_lounge_car(card *origin_card, player *origin)
            : request_base(origin_card, nullptr, origin) {}
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<2; ++i) {
                    if (card *drawn_card = target->m_game->top_train_card()) {
                        target->m_game->move_card(drawn_card, pocket_type::selection, target);
                    } else {
                        break;
                    }
                }
            }
            if (target->m_game->m_selection.size() <= 1 && rn::all_of(target->m_game->m_selection, [&](card *target_card) {
                return !filters::check_player_filter(target, target_card->equip_target, target);
            })) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    card *c = target->m_game->m_selection.front();
                    target->m_game->add_log("LOG_EQUIPPED_CARD", c, target);
                    target->equip_card(c);
                }
            }
        }

        bool in_target_set(const player *target_player) const override {
            return rn::any_of(target->m_game->m_selection, [&](card *target_card) {
                return !filters::check_player_filter(target, target_card->equip_target, target_player);
            });
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_LOUNGE_CAR", origin_card};
            } else {
                return {"STATUS_LOUNGE_CAR_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_lounge_car::on_prompt(card *origin_card, player *origin) {
        if (origin->m_game->top_train_card() == nullptr) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_lounge_car::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_lounge_car>(origin_card, origin);
    }

    bool effect_lounge_car_response::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_lounge_car>(origin) != nullptr;
    }

    game_string handler_lounge_car::get_error(card *origin_card, player *origin, card *target_card, player *target_player) {
        return filters::check_player_filter(origin, target_card->equip_target, target_player);
    }

    void handler_lounge_car::on_play(card *origin_card, player *origin, card *target_card, player *target_player) {
        origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", target_card, origin, target_player);
        target_player->equip_card(target_card);
    }
}