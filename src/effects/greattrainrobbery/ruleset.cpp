#include "ruleset.h"

#include "game/filters.h"

#include "next_stop.h"

#include "game/game.h"

namespace banggame {

    static void init_stations_and_train(player *origin) {
        origin->m_game->m_stations = origin->m_game->context().cards
            | rv::transform([](card &c) { return &c; })
            | rv::filter([](card *c) { return c->deck == card_deck_type::station; })
            | rv::sample(std::max(int(origin->m_game->m_players.size()), 4), origin->m_game->rng)
            | rn::to_vector;
            
        origin->m_game->add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(origin->m_game->m_stations), pocket_type::stations);
        for (card *c : origin->m_game->m_stations) {
            c->pocket = pocket_type::stations;
            origin->m_game->set_card_visibility(c, nullptr, card_visibility::shown, true);
        }

        origin->m_game->m_train = origin->m_game->context().cards
            | rv::transform([](card &c) { return &c; })
            | rv::filter([&](card *c) {
                return c->deck == card_deck_type::locomotive && !rn::contains(origin->m_game->m_train, c);
            })
            | rv::sample(1, origin->m_game->rng)
            | rn::to_vector;

        origin->m_game->add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(origin->m_game->m_train), pocket_type::train);
        for (card *c : origin->m_game->m_train) {
            c->pocket = pocket_type::train;
            origin->m_game->set_card_visibility(c, nullptr, card_visibility::shown, true);
            origin->enable_equip(c);
        }
        
        for (int i=0; i<3; ++i) {
            if (card *drawn_card = origin->m_game->top_train_card()) {
                origin->m_game->move_card(drawn_card, pocket_type::train);
            } else {
                break;
            }
        }
    }

    static void shuffle_stations_and_trains(player *origin) {
        while (origin->m_game->m_train.size() != 1) {
            origin->m_game->move_card(origin->m_game->m_train.back(), pocket_type::train_deck, nullptr, card_visibility::hidden);
        }
        
        origin->m_game->train_position = 0;
        origin->m_game->add_update<game_update_type::move_train>(0);

        for (card *c : origin->m_game->m_train) {
            origin->m_game->set_card_visibility(c, nullptr, card_visibility::hidden);
            origin->disable_equip(c);
        }
        origin->m_game->add_update<game_update_type::remove_cards>(rn::to<serial::card_list>(origin->m_game->m_train));

        if (!origin->m_game->m_train_deck.empty()) {
            origin->m_game->shuffle_cards_and_ids(origin->m_game->m_train_deck);
        }
        
        origin->m_game->add_update<game_update_type::remove_cards>(rn::to<serial::card_list>(origin->m_game->m_stations));
        for (card *c : origin->m_game->m_stations) {
            c->visibility = card_visibility::hidden;
        }
        
        init_stations_and_train(origin);
    }

    void ruleset_greattrainrobbery::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 1}, [](player *origin) {
            init_stations_and_train(origin);
        });

        game->add_listener<event_type::check_play_card>(nullptr, [](player *origin, card *origin_card, const effect_context &ctx, game_string &out_error) {
            if (filters::is_equip_card(origin_card) && origin_card->is_train()) {
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

        game->add_listener<event_type::on_equip_card>(nullptr, [](player *origin, player *target, card *origin_card, const effect_context &ctx) {
            if (origin_card->is_train()) {
                if (ctx.traincost->deck != card_deck_type::main_deck) {
                    event_card_key key{origin_card, 5};
                    origin->m_game->add_listener<event_type::count_train_equips>(key, [=](player *p, int &train_equips, int &num_advance) {
                        if (origin == p) {
                            ++train_equips;
                            num_advance += ctx.train_advance;
                        }
                    });
                    origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
                        if (origin == p) {
                            origin->m_game->remove_listeners(key);
                        }
                    });
                }

                if (card *drawn_card = origin->m_game->top_train_card()) {
                    origin->m_game->move_card(drawn_card, pocket_type::train);
                }
            }
        });

        game->add_listener<event_type::on_train_advance>(nullptr, [](player *origin, shared_effect_context ctx) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                for (int i=0; i < ctx->locomotive_count; ++i) {
                    origin->m_game->queue_action([=]{
                        origin->m_game->add_log("LOG_END_OF_LINE");
                        origin->m_game->call_event(event_type::on_locomotive_effect{ origin, ctx });
                    }, -1);
                }
                origin->m_game->queue_action([=]{
                    shuffle_stations_and_trains(origin);
                }, -6);
            }
        });

        game->add_listener<event_type::on_turn_switch>({nullptr, 1}, [](player *origin) {
            if (origin == origin->m_game->m_first_player) {
                origin->m_game->queue_action([=]{
                    origin->m_game->advance_train(origin);
                }, -5);
            }
        });
    }

}