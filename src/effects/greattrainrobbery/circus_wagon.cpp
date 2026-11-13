#include "circus_wagon.h"

#include "effects/base/pick.h"
#include "effects/ghost_cards/ruleset.h"

#include "game/game_table.h"
#include "game/bot_suggestion.h"

#include "cards/filter_enums.h"

namespace banggame {

    struct request_discard_table : request_picking {
        using request_picking::request_picking;

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target && !target_card->is_black();
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        int evaluate_pick(card_ptr target_card) const override {
            if (target_card->has_tag(tag_type::penalty)) {
                return 5;
            } else if (auto value = target_card->get_tag_value(tag_type::strong)) {
                return -5 - *value;
            } else if (is_ghost_card(target_card)) {
                return -10;
            }
            return 0;
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_DISCARD_TABLE", origin_card};
            } else {
                return {"STATUS_DISCARD_TABLE_OTHER", target, origin_card};
            }
        }
    };

    int effect_circus_wagon::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        if (bot_suggestion::is_target_enemy(origin, target)) {
            if (rn::any_of(target->m_table, [](card_ptr c) { return c->has_tag(tag_type::jail); })) {
                return -10;
            } else {
                return 4;
            }
        }
        return -2;
    }

    void effect_circus_wagon::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        origin->m_game->queue_request<request_discard_table>(origin_card, origin, target);
    }
}
