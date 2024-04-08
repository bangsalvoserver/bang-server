#ifndef __GAME_TABLE_H__
#define __GAME_TABLE_H__

#include <span>
#include <random>

#include "player.h"
#include "game_net.h"
#include "event_map.h"
#include "game_events.h"
#include "disabler_map.h"

namespace banggame {

    struct game_table : game_net_manager, listener_map, disabler_map {
        unsigned int rng_seed;
        std::default_random_engine rng;

        std::vector<player *> m_players;
        
        std::vector<card *> m_deck;
        std::vector<card *> m_discards;
        std::vector<card *> m_selection;

        std::vector<card *> m_shop_deck;
        std::vector<card *> m_shop_discards;
        std::vector<card *> m_hidden_deck;
        std::vector<card *> m_shop_selection;
        std::vector<card *> m_button_row;

        std::vector<card *> m_scenario_deck;
        std::vector<card *> m_scenario_cards;
        std::vector<card *> m_wws_scenario_deck;
        std::vector<card *> m_wws_scenario_cards;

        std::vector<card *> m_stations;
        std::vector<card *> m_train_deck;
        std::vector<card *> m_train;
        
        int8_t num_cubes = 0;
        int8_t train_position = 0;

        game_flags m_game_flags{};
        game_options m_options;

        player *m_first_player = nullptr;
        player *m_first_dead = nullptr;
        player *m_playing = nullptr;

        game_table(unsigned int seed);
        
        std::vector<card *> &get_pocket(pocket_type pocket, player *owner = nullptr);

        int calc_distance(const player *from, const player *to);

        int num_alive() const;

        void shuffle_cards_and_ids(std::span<card *> vec);
        card *add_card(const card_data &data);

        void set_card_visibility(card *c, player *owner = nullptr, card_visibility visibility = card_visibility::show_owner, bool instant = false);
        void move_card(card *c, pocket_type pocket, player *owner = nullptr, card_visibility visibility = card_visibility::show_owner, bool instant = false, bool front = false);

        card *top_of_deck();
        card *top_train_card();
        card *draw_shop_card();

        void draw_scenario_card();
        void advance_train(player *origin);

        void flash_card(card *c);
        void add_short_pause(card *c = nullptr);
        void tap_card(card *c, bool inactive);
        void play_sound(player *target, const std::string &sound_id);

        void add_cubes(card *target, int ncubes);
        void move_cubes(card *origin, card *target, int ncubes, bool instant = false);
        void drop_cubes(card *target);

        void add_game_flags(game_flags flags);
        void remove_game_flags(game_flags flags);
    
        bool check_flags(game_flags type) const;

        bool is_game_over() const;
    };

}

#endif