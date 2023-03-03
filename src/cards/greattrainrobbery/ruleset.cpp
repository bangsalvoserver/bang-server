#include "ruleset.h"

#include "cards/filters.h"

#include "next_stop.h"

#include "game/game.h"

namespace banggame {

    void ruleset_greattrainrobbery::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 1}, [=]{
            game->m_stations = game->m_context.cards
                | ranges::views::transform([](card &c) { return &c; })
                | ranges::views::filter([](card *c) { return c->deck == card_deck_type::station; })
                | ranges::views::sample(std::max(int(game->m_players.size()), 3), game->rng)
                | ranges::to<std::vector>;
                
            game->add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(game->m_stations), pocket_type::stations);
            for (card *c : game->m_stations) {
                c->pocket = pocket_type::stations;
                game->set_card_visibility(c, nullptr, card_visibility::shown, true);
            }

            game->m_train = game->m_context.cards
                | ranges::views::transform([](card &c) { return &c; })
                | ranges::views::filter([](card *c) { return c->deck == card_deck_type::locomotive; })
                | ranges::views::sample(1, game->rng)
                | ranges::to<std::vector>;

            game->add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(game->m_train), pocket_type::train);
            for (card *c : game->m_train) {
                c->pocket = pocket_type::train;
                game->set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
            
            for (int i=0; i<3; ++i) {
                game->move_card(game->m_train_deck.front(), pocket_type::train);
            }
        });

        game->add_listener<event_type::check_play_card>(nullptr, [](player *origin, card *origin_card, const effect_context &ctx, game_string &out_error) {
            if (filters::is_equip_card(origin_card) && origin_card->is_train()) {
                if (!ctx.traincost) {
                    out_error = "ERROR_MUST_PAY_TRAIN_COST";
                } else if (origin->m_game->call_event<event_type::count_train_equips>(origin, 0) >= 1) {
                    out_error = "ERROR_ONE_TRAIN_EQUIP_PER_TURN";
                }
            }
        });

        game->add_listener<event_type::on_equip_card>(nullptr, [](player *origin, player *target, card *origin_card, const effect_context &ctx) {
            if (origin_card->is_train()) {
                event_card_key key{origin_card, 5};
                origin->m_game->add_listener<event_type::count_train_equips>(key, [=](player *p, int &value) {
                    if (origin == p) {
                        ++value;
                    }
                });
                origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
                    if (origin == p) {
                        origin->m_game->remove_listeners(key);
                    }
                });

                if (!origin->m_game->m_train_deck.empty()) {
                    origin->m_game->move_card(origin->m_game->m_train_deck.front(), pocket_type::train);
                }
                if (ctx.train_advance) {
                    effect_next_stop{}.on_play(nullptr, origin);
                }
            }
        });

        game->add_listener<event_type::on_train_advance>(nullptr, [](player *origin) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                origin->m_game->queue_action([=]{
                    // TODO shuffle and recreate stations and train
                });
            }
        });
    }

}