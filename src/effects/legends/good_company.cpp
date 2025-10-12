#include "good_company.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static bool is_same_name(const_player_ptr origin, const_card_ptr target_card1, const_card_ptr target_card2) {
        if (origin->check_player_flags(player_flag::treat_missed_as_bang) || origin->check_player_flags(player_flag::treat_any_as_bang)) {
            auto is_bang_or_missed = [](const_card_ptr card) {
                return card->has_tag(tag_type::bangcard) || card->has_tag(tag_type::missedcard);
            };
            if (is_bang_or_missed(target_card1) || is_bang_or_missed(target_card2)) {
                return true;
            }
        }
        return target_card1->name == target_card2->name;
    }

    void equip_good_company::on_enable(card_ptr origin_card, player_ptr origin) {
        struct card_tracking {
            card_ptr last_played = nullptr;
            card_ptr last_discarded = nullptr;
        };

        auto tracking = std::make_shared<card_tracking>();

        origin->m_game->add_listener<event_type::on_turn_start>(origin_card, [=](player_ptr e_origin) {
            *tracking = {};
        });

        origin->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr e_origin, card_ptr target_card, bool is_destroyed, bool &handled) {
            if (e_origin == origin->m_game->m_playing && is_destroyed) {
                tracking->last_discarded = target_card;
            }
        });

        origin->m_game->add_listener<event_type::on_play_card>(origin_card, [=](player_ptr e_origin, card_ptr e_origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (e_origin == origin->m_game->m_playing && e_origin_card->deck == card_deck_type::main_deck) {
                tracking->last_played = e_origin_card;
                
                if (tracking->last_discarded && is_same_name(e_origin, tracking->last_discarded, tracking->last_played)) {
                    queue_request_perform_feat(origin_card, e_origin);
                } else {
                    tracking->last_discarded = nullptr;
                }
            }
        });

        origin->m_game->add_listener<event_type::check_target_set_card>(origin_card, [=](const_player_ptr e_origin, const_card_ptr target_card, bool &result) {
            result = tracking->last_played
                && target_card->pocket == pocket_type::player_hand
                && target_card->owner == origin->m_game->m_playing
                && is_same_name(e_origin, tracking->last_played, target_card);
        });
    }
}