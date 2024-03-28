#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#include "game_update.h"

namespace banggame {

    constexpr int max_cubes = 4;

    enum class card_visibility : uint8_t {
        hidden,
        shown,
        show_owner
    };
    
    struct card : card_data {
        card(int id, const card_data &data): card_data(data), order(id), id(id) {}
        
        const int order;
        int id;

        player *owner = nullptr;
        pocket_type pocket = pocket_type::none;
        card_visibility visibility = card_visibility::hidden;
        
        bool inactive = false;
        int8_t num_cubes = 0;
    };

    inline card_backface::card_backface(card *c): id(c->id), deck(c->deck) {}
    
    struct card_pocket_pair {
        card *origin_card;
        pocket_type pocket;
    };

    struct played_card_history {
        card_pocket_pair origin_card;
        std::vector<card_pocket_pair> modifiers;
        bool is_response;
        effect_context context;
    };

    enum class range_mod_type {
        range_mod,
        weapon_range,
        distance_mod
    };

    struct player {
        game *m_game;
        int id;
        int user_id;

        std::vector<card *> m_hand;
        std::vector<card *> m_table;
        std::vector<card *> m_characters;
        std::vector<card *> m_backup_character;

        player_role m_role;

        int8_t m_hp = 0;
        int8_t m_max_hp = 0;
        
        int8_t m_extra_turns = 0;

        std::vector<played_card_history> m_played_cards;

        player_flags m_player_flags{};

        int8_t m_gold = 0;

        player(game *game, int id) : m_game(game), id(id) {}

        void equip_card(card *card);
        card *find_equipped_card(card *card);

        void enable_equip(card *target_card);
        void disable_equip(card *target_card);
        
        card *random_hand_card();

        int can_escape(player *origin, card *origin_card, effect_flags flags) const;
        
        void add_to_hand(card *card);
        void draw_card(int ncards = 1, card *origin_card = nullptr);

        void discard_card(card *target, bool used = false);
        void discard_used_card(card *target) {
            discard_card(target, true);
        }
        void steal_card(card *target);

        int get_initial_cards();

        int max_cards_end_of_turn();

        int get_num_checks();
        int get_bangs_played();
        int get_range_mod() const;
        int get_weapon_range() const;
        int get_distance_mod() const;

        bool is_bot() const;
        bool is_ghost() const;
        bool alive() const;

        void damage(card *origin_card, player *source, int value, effect_flags flags = {});

        void heal(int value);
        void set_hp(int value, bool instant = false);

        void add_gold(int amount);

        bool immune_to(card *origin_card, player *origin, effect_flags flags, bool quiet = false);

        bool empty_table() const {
            return rn::all_of(m_table, &card::is_black);
        }

        bool empty_hand() const {
            return m_hand.empty();
        }

        card *first_character() const {
            if (!m_characters.empty()) {
                return m_characters.front();
            }
            return nullptr;
        }

        void set_role(player_role role, bool instant = true);
        void reset_max_hp();

        void start_of_turn();
        
        void pass_turn();
        void skip_turn();

        bool add_player_flags(player_flags flags);
        bool remove_player_flags(player_flags flags);
        bool check_player_flags(player_flags flags) const;

        int count_cubes() const;

        auto cube_slots() const {
            return rv::concat(
                m_table | rv::filter(&card::is_orange),
                rv::single(first_character())
            );
        }

        void remove_extra_characters();
    };

}

#endif