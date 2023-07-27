#include "switch_cards.h"

#include "cards/base/steal_destroy.h"

#include "cards/game_enums.h"
#include "cards/filters.h"

#include "game/game.h"

namespace banggame {

    static bool has_equipped_card(player *origin, card *target_card) {
        return std::ranges::any_of(origin->m_played_cards, [&](const card_pocket_pair &pair) {
            return pair.origin_card == target_card && pair.pocket == pocket_type::player_hand;
        }, &played_card_history::origin_card);
    }

    static void resolve_switch_cards(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        player *target = target_card->owner;
        origin->m_game->add_log("LOG_SWAP_CARDS", origin, target, chosen_card, target_card);

        target->disable_equip(target_card);
        origin->disable_equip(chosen_card);
        
        origin->m_game->move_card(target_card, pocket_type::player_table, origin, card_visibility::shown);
        if (target_card->is_green() && has_equipped_card(origin, target_card)) {
            origin->m_game->tap_card(target_card, true);
        }
        origin->m_game->tap_card(chosen_card, false);
        origin->m_game->move_card(chosen_card, pocket_type::player_table, target, card_visibility::shown);
        
        origin->enable_equip(target_card);
        target->enable_equip(chosen_card);
    }

    struct request_switch_cards : request_targeting {
        request_switch_cards(card *origin_card, player *origin, player *target, card *chosen_card, card *target_card)
            : request_targeting(origin_card, origin, target, target_card, effect_flags::escapable)
            , chosen_card(chosen_card) {}

        card *chosen_card;

        std::vector<card *> get_highlights() const override {
            return {target_card, chosen_card};
        }

        void on_resolve_target() override {
            resolve_switch_cards(origin_card, origin, chosen_card, target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_SWITCH_CARDS", origin_card, target_card, chosen_card};
            } else {
                return {"STATUS_SWITCH_CARDS_OTHER", target, origin_card, target_card, chosen_card};
            }
        }
    };

    game_string handler_switch_cards::get_error(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        player *target = target_card->owner;
        MAYBE_RETURN(filters::check_player_filter(origin, target_card->equip_target, origin));
        if (auto *c = origin->find_equipped_card(target_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        MAYBE_RETURN(filters::check_player_filter(target, chosen_card->equip_target, target));
        if (auto *c = target->find_equipped_card(chosen_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        return {};
    }

    void handler_switch_cards::on_play(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        origin->m_game->queue_request<request_switch_cards>(origin_card, origin, target_card->owner, chosen_card, target_card);
    }
}