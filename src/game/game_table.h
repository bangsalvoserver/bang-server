#ifndef __GAME_TABLE_H__
#define __GAME_TABLE_H__

#include <span>
#include <random>

#include "player.h"
#include "formatter.h"
#include "game_net.h"
#include "events.h"

#include "utils/id_map.h"

namespace banggame {

    using card_disabler_fun = std::function<bool(card *)>;

    struct game_table : game_net_manager<game_table> {
        std::default_random_engine rng;

        util::id_map<card> m_cards;
        util::id_map<player> m_players;
        
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
        
        int8_t num_cubes = 0;

        game_flags m_game_flags{};
        game_options m_options;

        player *m_first_player = nullptr;
        player *m_first_dead = nullptr;

        std::multimap<event_card_key, card_disabler_fun, std::less<>> m_disablers;

        game_table() {
            std::random_device rd;
            rng.seed(rd());
        }
        
        card *find_card(int card_id) const;
        player *find_player(int player_id) const;
        
        std::vector<card *> &get_pocket(pocket_type pocket, player *owner = nullptr);

        int calc_distance(player *from, player *to);

        int num_alive() const;

        void shuffle_cards_and_ids(std::span<card *> vec);

        void send_card_update(card *c, player *owner = nullptr, show_card_flags flags = {});

        void move_card(card *c, pocket_type pocket, player *owner = nullptr, show_card_flags flags = {});
        card *draw_card_to(pocket_type pocket, player *owner = nullptr, show_card_flags flags = {});
        card *draw_phase_one_card_to(pocket_type pocket, player *owner = nullptr, show_card_flags flags = {});
        card *phase_one_drawn_card();

        card *draw_shop_card();
        
        void draw_scenario_card();

        void flash_card(card *c);

        void add_disabler(event_card_key key, card_disabler_fun &&fun);
        void remove_disablers(event_card_key key);

        card *get_disabler(card *target_card);
        bool is_disabled(card *target_card);

        void set_game_flags(game_flags flags);
    
        bool check_flags(game_flags type) const {
            using namespace enums::flag_operators;
            return bool(m_game_flags & type);
        }
    };

}

#endif