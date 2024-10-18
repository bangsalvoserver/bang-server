#include "circus_wagon.h"

#include "effects/base/pick.h"

#include "game/game.h"

#include "cards/filter_enums.h"

namespace banggame {

    struct request_discard_table : request_picking {
        using request_picking::request_picking;

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target && !target_card->is_black();
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            if (target->is_bot()) {
                if (target_card->has_tag(tag_type::ghost_card) || target_card->self_equippable()) {
                    return "BOT_BAD_PLAY";
                }
            }
            return {};
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_DISCARD_TABLE", origin_card};
            } else {
                return {"STATUS_DISCARD_TABLE_OTHER", target, origin_card};
            }
        }
    };

    void effect_circus_wagon::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_discard_table>(origin_card, origin, target);
    }
}