#include "switch_cards.h"

#include "effects/base/steal_destroy.h"

#include "effects/dodgecity/ruleset.h"

#include "game/game_table.h"
#include "game/filters.h"
#include "game/prompts.h"

#include "cards/game_enums.h"

namespace banggame {

    static bool has_equipped_card(player_ptr origin, card_ptr target_card) {
        bool value = false;
        origin->m_game->call_event(event_type::check_equipped_green_card{ origin, target_card, value });
        return value;
    }

    static void resolve_switch_cards(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        player_ptr target = target_card->owner;
        origin->m_game->add_log("LOG_SWAP_CARDS", origin, target, chosen_card, target_card);

        target->disable_equip(target_card);
        origin->disable_equip(chosen_card);
        
        target_card->move_to(pocket_type::player_table, origin, card_visibility::shown);
        if (target_card->is_green() && has_equipped_card(origin, target_card)) {
            target_card->set_inactive(true);
        }
        chosen_card->set_inactive(false);
        chosen_card->move_to(pocket_type::player_table, target, card_visibility::shown);
        
        origin->enable_equip(target_card);
        target->enable_equip(chosen_card);
    }

    struct request_switch_cards : request_targeting, escapable_request {
        request_switch_cards(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr chosen_card, card_ptr target_card)
            : request_targeting(origin_card, origin, target, target_card, effect_flag::single_target)
            , chosen_card(chosen_card) {}

        card_ptr chosen_card;

        card_list get_highlights(player_ptr owner) const override {
            return {target_card, chosen_card};
        }

        void on_resolve() override {
            target->m_game->pop_request();
            resolve_switch_cards(origin_card, origin, chosen_card, target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_SWITCH_CARDS", origin_card, target_card, chosen_card};
            } else {
                return {"STATUS_SWITCH_CARDS_OTHER", target, origin_card, target_card, chosen_card};
            }
        }
    };

    game_string handler_switch_cards::get_error(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        player_ptr target = target_card->owner;
        MAYBE_RETURN(check_player_filter(target_card, origin, target_card->equip_target, origin));
        if (card_ptr c = origin->find_equipped_card(target_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        MAYBE_RETURN(check_player_filter(chosen_card, target, chosen_card->equip_target, target));
        if (card_ptr c = target->find_equipped_card(chosen_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        return {};
    }

    prompt_string handler_switch_cards::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_card(origin, target_card));
        return {};
    }

    void handler_switch_cards::on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        origin->m_game->queue_request<request_switch_cards>(origin_card, origin, target_card->owner, chosen_card, target_card);
    }
}