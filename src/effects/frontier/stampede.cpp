#include "stampede.h"

#include "effects/base/resolve.h"
#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/prompts.h"

namespace banggame {

    game_string effect_stampede::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        return prompts::bot_check_target_enemy(origin, target);
    }

    struct request_stampede : request_picking, interface_resolvable {
        using request_picking::request_picking;

        auto get_target_cards() const {
            return target->m_table | rv::remove_if(&card::is_black);
        }

        void on_update() override {
            if (update_count == 0 && target->immune_to(origin_card, origin, flags)) {
                pop_request();
            } else if (target->m_game->m_options.quick_discard_all || !contains_at_least(get_target_cards(), 2)) {
                on_resolve();
            }
        }

        void on_resolve() override {
            pop_request();

            while (auto cards = get_target_cards()) {
                on_pick(cards.front());
            }
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target
                && !target_card->is_black();
        }

        void on_pick(card_ptr target_card) override {
            target->discard_card(target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_STAMPEDE", origin_card};
            } else {
                return {"STATUS_STAMPEDE_OTHER", target, origin_card};
            }
        }
    };

    void effect_stampede::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_stampede>(origin_card, origin, target);
    }
}