#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_update.h"

namespace banggame {

    struct player {
        game_ptr m_game;
        int id;

        int user_id = 0;

        card_list m_hand;
        card_list m_table;
        card_list m_characters;

        player_role m_role;

        int8_t m_hp = 0;
        int8_t m_max_hp = 0;
        
        int8_t m_extra_turns = 0;

        token_map tokens;

        player_flags m_player_flags;

        player(game_ptr game, int id): m_game(game), id(id) {}

        void equip_card(card_ptr card, bool skip_enable = false);
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

        void remove_characters(card_ptr start_from = nullptr, bool exclude_first = false);
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

        void reveal_hand();
    };

    inline int get_player_id(const_player_ptr target) {
        return target ? target->id : 0;
    }

    class player_set {
    private:
        uint16_t m_value = 0;

        player_set(uint16_t value)
            : m_value{value} {}

        player_set(bool exclusive, std::convertible_to<const_player_ptr> auto ... targets)
            : m_value{static_cast<uint16_t>(exclusive)}
        {
            (add(targets), ...);
        }

        static uint16_t get_player_bit(const_player_ptr target) {
            return 1 << target->id;
        }

    public:
        player_set() = default;

        static player_set includes(std::convertible_to<const_player_ptr> auto ... targets) {
            return player_set(false, targets...);
        }

        static player_set excludes(std::convertible_to<const_player_ptr> auto ... targets) {
            return player_set(true, targets...);
        }

        void add(const_player_ptr target) {
            m_value |= get_player_bit(target);
        }

        void remove(const_player_ptr target) {
            m_value &= ~get_player_bit(target);
        }

        bool contains(const_player_ptr target) const {
            return target && bool(m_value & get_player_bit(target));
        }

        bool matches(const_player_ptr target) const {
            return contains(target) != exclusive();
        }

        bool exclusive() const {
            return bool(m_value & 1);
        }

        explicit operator bool() const {
            return m_value != 0;
        }

        player_set operator & (const player_set &rhs) const {
            return player_set{static_cast<uint16_t>(m_value & rhs.m_value)};
        }

        player_set operator | (const player_set &rhs) const {
            return player_set{static_cast<uint16_t>(m_value | rhs.m_value)};
        }

        player_set operator - (const player_set &rhs) const {
            return player_set{static_cast<uint16_t>(m_value & ~rhs.m_value)};
        }
    };

}

#endif