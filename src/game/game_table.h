#ifndef __GAME_TABLE_H__
#define __GAME_TABLE_H__

#include <span>
#include <random>

#include "utils/id_map.h"

#include "player.h"
#include "game_net.h"
#include "event_map.h"
#include "disabler_map.h"
#include "request_queue.h"
#include "utils/range_utils.h"

namespace banggame {

    struct game_table : game_net_manager, listener_map, disabler_map, request_queue {
        const game_options &m_options;
        
        unsigned int rng_seed;
        std::default_random_engine rng;
        std::default_random_engine bot_rng;

        utils::id_map<card> m_cards_storage;
        utils::id_map<player> m_players_storage;

        player_list m_players;

        card_list m_characters;
        card_list m_legends;
        card_list m_copies;
        
        card_list m_deck;
        card_list m_discards;
        card_list m_selection;

        card_list m_shop_deck;
        card_list m_hidden_deck;
        card_list m_shop_selection;
        card_list m_button_row;

        card_list m_scenario_deck;
        card_list m_scenario_cards;
        card_list m_wws_scenario_deck;
        card_list m_wws_scenario_cards;

        card_list m_stations_deck;
        card_list m_stations;
        
        card_list m_locomotive;
        card_list m_train_deck;
        card_list m_train;

        card_list m_feats_deck;
        card_list m_feats_discard;
        card_list m_feats;
        
        token_map tokens;
        int8_t train_position = 0;

        game_flags m_game_flags;

        player_ptr m_first_player = nullptr;
        player_ptr m_playing = nullptr;

        game_table(const game_options &options);

        card_ptr add_card(const card_data &data);
        void add_players(std::span<int> user_ids);

        void remove_cards(card_list cards);
        void add_cards_to(card_list cards, pocket_type pocket, player_ptr owner = nullptr, card_visibility visibility = card_visibility::hidden, bool instant = true);

        card_ptr find_card(int card_id) const override;
        player_ptr find_player(int player_id) const override;
        player_ptr find_player_by_userid(int user_id) const override;
        game_duration transform_duration(game_duration duration) const override;
        
        card_list &get_pocket(pocket_type pocket, player_ptr owner = nullptr);

        auto range_all_players(const_player_ptr begin) const {
            return rotate_range(m_players, rn::find(m_players, begin));
        }

        auto range_alive_players(const_player_ptr begin) const {
            return range_all_players(begin) | rv::filter(&player::alive);
        }

        auto range_other_players(const_player_ptr begin) const {
            return range_all_players(begin) | rv::drop(1) | rv::filter(&player::alive);
        }

        int calc_distance(const_player_ptr from, const_player_ptr to) const;

        int num_alive() const;

        void shuffle_cards_and_ids(std::span<card_ptr> vec);

        int num_tokens(card_token_type token_type) const;
        void add_tokens(card_token_type token_type, int num_tokens, card_ptr target_card = nullptr);

        void clear_request_status();

        card_ptr top_of_deck();

        void add_short_pause();
        void play_sound(std::string_view sound_id);
        
        void start_next_turn();

        void add_game_flags(game_flag flags);
        void remove_game_flags(game_flag flags);
        bool check_flags(game_flag type) const;

        bool is_game_over() const override;
    };

}

#endif