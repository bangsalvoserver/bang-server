#include "ruleset.h"

#include "effects/base/death.h"
#include "effects/ghost_cards/ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/expansion_set.h"

namespace banggame {

    int count_played_cards(card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
        auto is_played_card = [](card_ptr c) {
            switch (c->pocket) {
            case pocket_type::player_hand:
            case pocket_type::shop_selection:
                return true;
            case pocket_type::train:
                return c->deck == card_deck_type::train;
            default:
                return false;
            }
        };
        
        return rn::count_if(modifiers, is_played_card)
            + (origin_card != ctx.get<contexts::playing_card>() || is_played_card(origin_card));
    }

    void track_played_cards(game_ptr game) {
        event_card_key key{nullptr, -15};

        game->add_listener<event_type::on_turn_start>(nullptr, [=](player_ptr origin) {
            origin->m_game->remove_listeners(key);
        });

        game->add_listener<event_type::on_play_card>(nullptr, [=](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (origin == origin->m_game->m_playing) {
                if (int count = count_played_cards(origin_card, modifiers, ctx)) {
                    origin->m_game->add_listener<event_type::get_count_played_cards>(key, [=](player_ptr e_origin, int &value) {
                        if (origin == e_origin) {
                            value += count;
                        }
                    });
                }
            }
        });
    }

    int get_count_played_cards(player_ptr origin) {
        int count = 0;
        origin->m_game->call_event(event_type::get_count_played_cards{ origin, count });
        return count;
    }

    void ruleset_wildwestshow::on_apply(game_ptr game) {
        track_played_cards(game);
        
        if (game->m_options.expansions.contains(GET_RULESET(ghost_cards))) {
            game->add_listener<event_type::check_remove_player>(nullptr, []{ return true; });
        }
    }
}