#include "leevankliff.h"

#include "effects/base/bang.h"
#include "effects/base/card_choice.h"
#include "effects/greattrainrobbery/traincost.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    namespace event_type {
        struct get_last_played_brown_card {
            player_ptr origin;
            nullable_ref<card_ptr> value;
        };
    }

    card_ptr get_repeat_playing_card(card_ptr origin_card, const effect_context &ctx) {
        if (card_ptr card_choice = ctx.get<contexts::card_choice>()) {
            return card_choice;
        } else if (card_ptr traincost = ctx.get<contexts::train_cost>()) {
            return traincost;
        } else {
            return origin_card;
        }
    }
    
    void equip_leevankliff::on_enable(card_ptr origin_card, player_ptr origin) {
        auto last_played = std::make_shared<card_ptr>(nullptr);

        origin->m_game->add_listener<event_type::on_turn_start>(origin_card, [=](player_ptr e_origin) {
            if (origin == e_origin) {
                *last_played = nullptr;
            }
        });

        origin->m_game->add_listener<event_type::get_last_played_brown_card>(origin_card, [=](player_ptr e_origin, card_ptr &value) {
            if (origin == e_origin) {
                value = *last_played;
            }
        });

        origin->m_game->add_listener<event_type::on_play_card>(origin_card, [=](player_ptr e_origin, card_ptr e_origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (origin == e_origin) {
                if (rn::contains(modifiers, origin_card)) {
                    *last_played = nullptr;
                } else if (card_ptr target_card = get_repeat_playing_card(e_origin_card, ctx);
                    target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::shop_selection
                ) {
                    if (target_card->is_brown()) {
                        *last_played = target_card;
                    } else {
                        // does "the brown-bordered card he just played" mean "the latest brown card he played"
                        // or "the latest card he played if it is brown"?
                        *last_played = nullptr;
                    }
                }
            }
        });
    }

    game_string modifier_leevankliff::get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) {
        card_ptr repeat_card = ctx.get<contexts::repeat_card>();
        if (!repeat_card) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        playing_card = get_repeat_playing_card(playing_card, ctx);
                
        if (repeat_card != playing_card) {
            return "INVALID_MODIFIER_CARD";
        }

        if (!playing_card->is_brown()) {
            return "ERROR_CARD_IS_NOT_BROWN";
        }

        return {};
    }

    void modifier_leevankliff::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        card_ptr last_played = nullptr;
        origin->m_game->call_event(event_type::get_last_played_brown_card{ origin, last_played });
        if (last_played) {
            ctx.add<contexts::disable_banglimit>();
            ctx.set<contexts::repeat_card>(last_played);
            ctx.set<contexts::playing_card>(origin_card);
        }
    }
}