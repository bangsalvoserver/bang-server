#include "thedaltons.h"

#include "effects/base/pick.h"

#include "game/game.h"

namespace banggame {

    struct request_thedaltons : request_picking {
        request_thedaltons(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table
                && target_card->owner == target
                && target_card->is_blue();
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_THEDALTONS", origin_card};
            } else {
                return {"STATUS_THEDALTONS_OTHER", target, origin_card};
            }
        }
    };

    void equip_thedaltons::on_enable(card_ptr target_card, player_ptr target) {
        for (player_ptr p : range_alive_players(target)) {
            if (rn::any_of(p->m_table, &card::is_blue)) {
                p->m_game->queue_request<request_thedaltons>(target_card, p);
            }
        }
    }
}