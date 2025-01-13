#include "ruleset.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "effects/base/damage.h"
#include "effects/base/death.h"

#include "game/game_table.h"

namespace banggame {

    static bool is_dynamite(card_ptr origin_card) {
        return origin_card->has_tag(tag_type::dynamite);
    }

    static void shuffle_card_back(card_ptr origin_card) {
        origin_card->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden, false, pocket_position::random);
    }

    static player_ptr find_dynamite_stick(game_ptr game) {
        auto it = rn::find_if(game->m_players, [](player_ptr p) {
            return p->check_player_flags(player_flag::stick_of_dynamite);
        });
        if (it != game->m_players.end()) {
            return *it;
        }
        return nullptr;
    }

    void ruleset_stickofdynamite::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 20}, [=](player_ptr origin){
            origin->add_player_flags(player_flag::stick_of_dynamite);

            for (player_ptr p : origin->m_game->range_alive_players(origin)) {
                while (auto filter = p->m_hand | rv::filter(is_dynamite)) {
                    card_ptr target_card = rn::front(filter);

                    game->add_log("LOG_REVEALED_CARD", p, target_card);
                    target_card->set_visibility(card_visibility::shown);
                    target_card->add_short_pause();

                    shuffle_card_back(target_card);
                    p->draw_card();
                }
            }
            
            game->add_listener<event_type::on_drawn_any_card>(nullptr, [=, recurse_count = 0](card_ptr &drawn_card) mutable {
                if (is_dynamite(drawn_card)) {
                    ++recurse_count;
                    if (recurse_count > 10) {
                        throw game_error("recursion limit hit in stickofdynamite on_drawn_any_card");
                    }

                    game->add_log("LOG_REVEALED_CARD", static_cast<player_ptr>(nullptr), drawn_card);
                    drawn_card->set_visibility(card_visibility::shown);
                    drawn_card->add_short_pause();

                    player_ptr target = find_dynamite_stick(game);
                    if (!target) {
                        target = game->m_playing;
                    }
                    if (target && target->alive() && !target->find_equipped_card(drawn_card)) {
                        target->add_player_flags(player_flag::stick_of_dynamite);
                        target->equip_card(drawn_card);
                    } else if (game->m_deck.size() <= 1) {
                        drawn_card->move_to(pocket_type::discard_pile);
                    } else {
                        shuffle_card_back(drawn_card);
                    }

                    drawn_card = game->top_of_deck();

                    --recurse_count;
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

        game->add_listener<event_type::on_player_eliminated>({nullptr, 20}, [=](player_ptr killer, player_ptr target, death_type death) {
            target->remove_player_flags(player_flag::stick_of_dynamite);
        });

        game->add_listener<event_type::on_discard_any_card>(nullptr, [](player_ptr origin, card_ptr target_card) {
            if (is_dynamite(target_card)) {
                target_card->add_short_pause();
                shuffle_card_back(target_card);
            }
        });
    }
}