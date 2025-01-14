#include "ruleset.h"

#include "next_stop.h"

#include "effects/base/death.h"

#include "cards/game_events.h"

#include "game/filters.h"
#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    static void init_stations_and_train(player_ptr origin) {
        origin->m_game->add_cards_to(origin->m_game->m_stations_deck
            | rv::sample(std::max(int(origin->m_game->m_players.size()), 4), origin->m_game->rng)
            | rn::to_vector, pocket_type::stations, nullptr, card_visibility::shown);

        origin->m_game->add_cards_to(origin->m_game->m_locomotive
            | rv::filter([](card_ptr c) { return c->pocket != pocket_type::train; })
            | rv::sample(1, origin->m_game->rng)
            | rn::to_vector, pocket_type::train, nullptr, card_visibility::shown);

        for (card_ptr c : origin->m_game->m_train) {
            origin->enable_equip(c);
        }
        
        for (int i=0; i<3 && !origin->m_game->m_train_deck.empty(); ++i) {
            origin->m_game->m_train_deck.back()->move_to(pocket_type::train);
        }
    }

    static void shuffle_stations_and_trains(player_ptr origin) {
        while (origin->m_game->m_train.size() != 1) {
            origin->m_game->m_train.back()->move_to(pocket_type::train_deck, nullptr, card_visibility::hidden);
        }
        
        origin->m_game->train_position = 0;
        origin->m_game->add_update<"move_train">(0);

        if (!origin->m_game->m_train_deck.empty()) {
            origin->m_game->shuffle_cards_and_ids(origin->m_game->m_train_deck);
            origin->m_game->add_log("LOG_TRAIN_RESHUFFLED");
            origin->m_game->play_sound("shuffle");
            origin->m_game->add_update<"deck_shuffled">(pocket_type::train_deck);
        }

        for (card_ptr target_card : origin->m_game->m_train) {
            origin->disable_equip(target_card);
        }

        origin->m_game->remove_cards(origin->m_game->m_train);
        origin->m_game->remove_cards(origin->m_game->m_stations);
        
        init_stations_and_train(origin);
    }

    void ruleset_greattrainrobbery::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 1}, init_stations_and_train);

        game->add_listener<event_type::check_play_card>(nullptr, [](player_ptr origin, card_ptr origin_card, const effect_context &ctx, game_string &out_error) {
            if (origin_card->is_equip_card() && origin_card->is_train()) {
                if (!ctx.traincost) {
                    out_error = "ERROR_MUST_PAY_TRAIN_COST";
                } else if (ctx.traincost->deck != card_deck_type::main_deck) {
                    int train_equips = 0;
                    int num_advance = 0;
                    origin->m_game->call_event(event_type::count_train_equips{ origin, train_equips, num_advance });
                    if (train_equips >= 1) {
                        out_error = "ERROR_ONE_TRAIN_EQUIP_PER_TURN";
                    } else if (ctx.train_advance >= 1 && num_advance >= 1) {
                        out_error = "ERROR_ONE_TRAIN_ADVANCE_PER_TURN";
                    }
                }
            }
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player_ptr origin, player_ptr target, card_ptr origin_card, const effect_context &ctx) {
            if (origin_card->is_train()) {
                if (ctx.traincost->deck != card_deck_type::main_deck) {
                    event_card_key key{origin_card, 5};
                    origin->m_game->add_listener<event_type::count_train_equips>(key, [=](player_ptr p, int &train_equips, int &num_advance) {
                        if (origin == p) {
                            ++train_equips;
                            num_advance += ctx.train_advance;
                        }
                    });
                    origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr p, bool skipped) {
                        if (origin == p) {
                            origin->m_game->remove_listeners(key);
                        }
                    });
                }

                if (!origin->m_game->m_train_deck.empty()) {
                    origin->m_game->m_train_deck.back()->move_to(pocket_type::train);
                }
            }
        });

        game->add_listener<event_type::on_train_advance>(nullptr, [](player_ptr origin, shared_locomotive_context ctx) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                for (int i=0; i < ctx->locomotive_count; ++i) {
                    origin->m_game->queue_action([=]{
                        origin->m_game->add_log("LOG_END_OF_LINE");
                        origin->m_game->call_event(event_type::on_locomotive_effect{ origin, ctx });
                    }, -1);
                }
                origin->m_game->queue_action([=]{
                    shuffle_stations_and_trains(origin);
                }, -15);
            }
        });

        game->add_listener<event_type::on_turn_switch>({nullptr, 1}, [](player_ptr origin) {
            if (origin == origin->m_game->m_first_player) {
                origin->m_game->queue_action([=]{
                    effect_next_stop{}.on_play(nullptr, origin);
                }, -10);
            }
        });
        
        if (game->m_options.enable_ghost_cards) {
            game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) { value = false; });
        }
    }

}