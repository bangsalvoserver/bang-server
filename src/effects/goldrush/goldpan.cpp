#include "goldpan.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    game_string effect_pay_gold::get_error(card_ptr origin_card, player_ptr origin) {
        if (origin->get_gold() < amount) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        return {};
    }

    void effect_pay_gold::on_play(card_ptr origin_card, player_ptr origin) {
        origin->add_gold(-amount);
    }

    static void drop_all_gold(card_ptr target_card) {
        if (int gold = target_card->num_tokens(card_token_type::gold)) {
            target_card->m_game->move_tokens(card_token_type::gold, token_positions::card{target_card}, token_positions::table{}, gold);
        }
    }

    void equip_goldpan::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr e_origin, bool skipped) {
            if (e_origin == target) {
                drop_all_gold(target_card);
            }
        });
    }

    void equip_goldpan::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(target_card);
        drop_all_gold(target_card);
    }

    game_string effect_goldpan::get_error(card_ptr origin_card, player_ptr origin) {
        if (origin->get_gold() < 1) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        if (origin_card->num_tokens(card_token_type::gold) >= max_usages) {
            return {"ERROR_MAX_USAGES", origin_card, max_usages};
        }
        return {};
    }

    void effect_goldpan::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->move_tokens(card_token_type::gold, token_positions::player{origin}, token_positions::card{origin_card}, 1);
    }
}