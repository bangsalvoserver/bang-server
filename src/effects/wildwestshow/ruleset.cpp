#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void ruleset_wildwestshow::on_apply(game *game) {
        event_card_key key{nullptr, -15};

        game->add_listener<event_type::on_turn_start>(nullptr, [=](player_ptr origin) {
            origin->m_game->remove_listeners(key);
        });

        game->add_listener<event_type::on_play_card>(nullptr, [=](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (origin == origin->m_game->m_playing) {
                auto is_playing_card = [](card_ptr origin_card) {
                    return origin_card->pocket == pocket_type::player_hand
                        || origin_card->pocket == pocket_type::shop_selection
                        || (origin_card->pocket == pocket_type::train && origin_card->deck == card_deck_type::train);
                };
                
                int count = rn::count_if(modifiers, is_playing_card);
                if (origin_card != ctx.playing_card || is_playing_card(origin_card)) {
                    ++count;
                }

                if (count != 0) {
                    origin->m_game->add_listener<event_type::get_count_played_cards>(key, [=](player_ptr e_origin, int &value) {
                        if (origin == e_origin) {
                            value += count;
                        }
                    });
                }
            }
        });
    }
}