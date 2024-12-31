#include "ruleset.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "effects/base/damage.h"
#include "effects/base/deathsave.h"

#include "game/game.h"

namespace banggame {

    static bool is_dynamite(card_ptr origin_card) {
        return origin_card->has_tag(tag_type::dynamite);
    }

    static void shuffle_card_back(card_ptr origin_card) {
        origin_card->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden, false, pocket_position::random);
    }

    static player_ptr find_dynamite_stick(game *game) {
        auto it = rn::find_if(game->m_players, [](player_ptr p) {
            return p->check_player_flags(player_flag::stick_of_dynamite);
        });
        if (it != game->m_players.end()) {
            return *it;
        }
        return nullptr;
    }

    void ruleset_stickofdynamite::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 20}, [=](player_ptr origin){
            origin->add_player_flags(player_flag::stick_of_dynamite);

            for (player_ptr p : origin->m_game->range_alive_players(origin)) {
                while (auto filter = p->m_hand | rv::filter(is_dynamite)) {
                    card_ptr target_card = rn::front(filter);
                    target_card->add_short_pause();
                    shuffle_card_back(target_card);
                    p->draw_card();
                }
            }
            
            game->add_listener<event_type::on_drawn_any_card>(nullptr, [=](card_ptr &drawn_card) {
                if (is_dynamite(drawn_card)) {
                    game->add_log("LOG_REVEALED_CARD", static_cast<player_ptr>(nullptr), drawn_card);
                    drawn_card->set_visibility(card_visibility::shown);
                    drawn_card->add_short_pause();

                    player_ptr target = find_dynamite_stick(game); 
                    if (target && target->alive() && !target->find_equipped_card(drawn_card)) {
                        target->equip_card(drawn_card);
                    } else {
                        shuffle_card_back(drawn_card);
                    }

                    drawn_card = game->top_of_deck();
                }
            });
        });

        game->add_listener<event_type::on_hit>({nullptr, 20}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) { 
            if (origin && flags.check(effect_flag::is_bang)) {
                player_ptr found = find_dynamite_stick(game);

                if (found && found != origin) {
                    found->remove_player_flags(player_flag::stick_of_dynamite);
                }
                
                origin->add_player_flags(player_flag::stick_of_dynamite);
            }
        });

        game->add_listener<event_type::on_player_eliminated>({nullptr, 20}, [=](player_ptr killer, player_ptr target) {
            if (target->remove_player_flags(player_flag::stick_of_dynamite) && game->m_first_player->alive()) {
                game->m_first_player->add_player_flags(player_flag::stick_of_dynamite);
            }
        });

        game->add_listener<event_type::on_discard_any_card>(nullptr, [](player_ptr origin, card_ptr target_card) {
            if (is_dynamite(target_card)) {
                target_card->add_short_pause();
                shuffle_card_back(target_card);
            }
        });
    }
}