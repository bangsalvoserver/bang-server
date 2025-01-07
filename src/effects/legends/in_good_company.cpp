#include "in_good_company.h"

#include "perform_feat.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    namespace event_type {
        struct get_last_discarded_card {
            nullable_ref<card_ptr> value;
        };
    }

    static card_ptr get_last_played_card(player_ptr origin) {
        for (const played_card_history &history : origin->m_played_cards | rv::reverse) {
            if (history.origin_card.pocket == pocket_type::player_hand) {
                return history.origin_card.origin_card;
            }
        }
        return nullptr;
    }

    static bool is_same_name(player_ptr origin, card_ptr target_card1, card_ptr target_card2) {
        if (origin->check_player_flags(player_flag::treat_missed_as_bang) || origin->check_player_flags(player_flag::treat_any_as_bang)) {
            auto is_bang_or_missed = [](card_ptr card) {
                return card->has_tag(tag_type::bangcard) || card->has_tag(tag_type::missedcard);
            };
            if (is_bang_or_missed(target_card1) || is_bang_or_missed(target_card2)) {
                return true;
            }
        }
        return target_card1->name == target_card2->name;
    }

    void equip_in_good_company::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(origin_card, [=, last_discarded = static_cast<card_ptr>(nullptr)](player_ptr origin) mutable {
            last_discarded = nullptr;

            event_card_key key{origin_card, 5};

            target->m_game->add_listener<event_type::get_last_discarded_card>(key, [&last_discarded](card_ptr &value) {
                value = last_discarded;
            });

            target->m_game->add_listener<event_type::on_destroy_card>(key, [=, &last_discarded](player_ptr e_origin, card_ptr target_card, bool is_destroyed, bool &handled) {
                if (e_origin == origin && is_destroyed) {
                    last_discarded = target_card;
                    if (card_ptr last_played = get_last_played_card(origin)) {
                        if (is_same_name(origin, last_discarded, last_played)) {
                            queue_request_perform_feat(origin_card, origin);
                        }
                    }
                }
            });

            target->m_game->add_listener<event_type::on_play_card>(key, [=, &last_discarded](player_ptr e_origin, card_ptr e_origin_card) {
                if (e_origin == origin && e_origin_card->deck == card_deck_type::main_deck) {
                    if (last_discarded && is_same_name(origin, last_discarded, e_origin_card)) {
                        queue_request_perform_feat(origin_card, origin);
                    } else {
                        last_discarded = nullptr;
                    }
                }
            });

            target->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr origin, bool skipped) {
                target->m_game->remove_listeners(key);
            });
        });

    }

    game_string effect_in_good_company::get_error(card_ptr origin_card, player_ptr origin, card_ptr target) {
        MAYBE_RETURN(effect_discard::get_error(origin_card, origin, target));

        if (card_ptr last_played = get_last_played_card(origin)) {
            if (is_same_name(origin, last_played, target)) {
                return {};
            }
        }
        return {"ERROR_CANT_PLAY_CARD", origin_card};
    }
}