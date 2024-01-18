#include "lastwill.h"

#include "game/game.h"

#include "cards/base/deathsave.h"

namespace banggame {

    struct request_lastwill : request_base {
        request_lastwill(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_update() override {
            if (target->m_hp > 0) {
                target->m_game->pop_request();
            }
        }
        
        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_LASTWILL", origin_card};
            } else {
                return {"STATUS_LASTWILL_OTHER", origin_card, target};
            }
        }
    };

    void equip_lastwill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>({origin_card, -1}, [=](player *target, bool tried_save) {
            if (origin == target) {
                origin->m_game->queue_request<request_lastwill>(origin_card, origin);
            }
        });
    }

    bool handler_lastwill::on_check_target(card *origin_card, player *origin, opt_tagged_value<target_type::max_cards> target_cards, std::optional<player *> target) {
        return !target || bot_suggestion::target_friend{}.on_check_target(origin_card, origin, *target);
    }

    game_string handler_lastwill::get_error(card *origin_card, player *origin, opt_tagged_value<target_type::max_cards> target_cards, std::optional<player *> target) {
        if (origin->m_game->top_request<request_lastwill>(origin) != nullptr) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    void handler_lastwill::on_play(card *origin_card, player *origin, opt_tagged_value<target_type::max_cards> target_cards, std::optional<player *> target_player) {
        origin->m_game->pop_request();
        if (!target_player) {
            return;
        }
        player *target = *target_player;
        for (card *chosen_card : target_cards->value) {
            if (chosen_card->visibility != card_visibility::shown) {
                origin->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
                origin->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
            } else {
                origin->m_game->add_log("LOG_GIFTED_CARD", origin, target, chosen_card);
            }
            target->steal_card(chosen_card);
        }
    }
}