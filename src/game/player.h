#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_update.h"

namespace banggame {

    struct player {
        game_ptr m_game;
        int id;
        int user_id;

        card_list m_hand;
        card_list m_table;
        card_list m_characters;

        rn::concat_view<
            rn::ref_view<card_list>,
            rn::ref_view<card_list>,
            rn::ref_view<card_list>
        > m_targetable_cards_view = rv::concat(m_hand, m_table, m_characters);

        player_role m_role;

        int8_t m_hp = 0;
        int8_t m_max_hp = 0;
        
        int8_t m_extra_turns = 0;

        token_map tokens;

        player_flags m_player_flags;

        player(game_ptr game, int id, int user_id)
            : m_game(game), id(id), user_id(user_id) {}

        void equip_card(card_ptr card);
        card_ptr find_equipped_card(const_card_ptr card) const;

        void enable_equip(card_ptr target_card);
        void disable_equip(card_ptr target_card);

        void play_sound(sound_id sound);
        
        card_ptr random_hand_card() const;
        
        void add_to_hand(card_ptr card);
        void draw_card(int ncards = 1, card_ptr origin_card = nullptr);

        void discard_card(card_ptr target, bool used = false);
        void discard_used_card(card_ptr target) {
            discard_card(target, true);
        }
        void steal_card(card_ptr target);

        int max_cards_end_of_turn() const;

        int get_num_checks() const;
        int get_bangs_played() const;
        int get_range_mod() const;
        int get_weapon_range() const;
        int get_distance_mod() const;
        
        player_ptr get_next_player() const;
        player_ptr get_prev_player() const;

        bool is_bot() const;
        bool is_ghost() const;
        bool alive() const;

        void damage(card_ptr origin_card, player_ptr source, int value, effect_flags flags = {});

        void heal(int value);
        void set_hp(int value, bool instant = false);

        int get_gold() const;
        void add_gold(int amount);

        bool immune_to(card_ptr origin_card, player_ptr origin, effect_flags flags, bool quiet = false) const;

        bool empty_table() const;
        bool empty_hand() const {
            return m_hand.empty();
        }

        card_ptr get_character() const {
            if (!m_characters.empty()) {
                return m_characters.front();
            }
            return nullptr;
        }

        void set_character(card_ptr target_card);
        
        int get_character_max_hp() const;

        void set_role(player_role role, bool instant = false);
        player_role get_base_role() const;

        void start_of_turn();
        
        void pass_turn();
        void skip_turn();

        bool add_player_flags(player_flag flags);
        bool remove_player_flags(player_flag flags);
        bool check_player_flags(player_flag flags) const;

        void remove_cards(card_list cards);
        void reveal_hand();
    };

}

#endif