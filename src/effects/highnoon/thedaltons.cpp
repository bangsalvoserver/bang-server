#include "thedaltons.h"

#include "effects/base/pick.h"
#include "effects/ghost_cards/ruleset.h"

#include "game/game_table.h"

#include "cards/filter_enums.h"

namespace banggame {

    struct request_thedaltons : request_picking {
        request_thedaltons(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table
                && target_card->owner == target
                && target_card->is_blue();
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        int evaluate_pick(card_ptr target_card) const override {
            if (target_card->has_tag(tag_type::penalty)) {
                return 5;
            }
            if (auto value = target_card->get_tag_value(tag_type::strong)) {
                return -5 - *value;
            }
            if (is_ghost_card(target_card)) {
                return -10;
            }
            return 0;
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_THEDALTONS", origin_card};
            } else {
                return {"STATUS_THEDALTONS_OTHER", target, origin_card};
            }
        }
    };

    void equip_thedaltons::on_enable(card_ptr target_card, player_ptr target) const {
        for (player_ptr p : target->m_game->range_alive_players(target)) {
            if (rn::any_of(p->m_table, &card::is_blue)) {
                p->m_game->queue_request<request_thedaltons>(target_card, p);
            }
        }
    }
}
