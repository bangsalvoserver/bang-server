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
    
    struct card : card_data {
        int id;
        
        bool inactive = false;
        int8_t num_cubes = 0;

        pocket_type pocket = pocket_type::none;
        player *owner = nullptr;

        void on_equip(player *target) {
            for (auto &e : equips) {
                e.on_equip(this, target);
            }
        }

        void on_enable(player *target) {
            for (auto &e : equips) {
                e.on_enable(this, target);
            }
        }

        void on_disable(player *target) {
            for (auto &e : equips) {
                e.on_disable(this, target);
            }
        }

        void on_unequip(player *target) {
            for (auto &e : equips) {
                e.on_unequip(this, target);
            }
        }
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

        struct predraw_check {
            int priority;
            bool resolved;
        };

        std::map<card *, predraw_check> m_predraw_checks;

        std::optional<std::pair<std::function<void()>, game_string>> m_prompt;

        int8_t m_range_mod = 0;
        int8_t m_weapon_range = 1;
        int8_t m_distance_mod = 0;

        int8_t m_hp = 0;
        int8_t m_max_hp = 0;
        
        int8_t m_num_drawn_cards = 0;
        
        int8_t m_extra_turns = 0;

        card *m_last_played_card = nullptr;

        player_flags m_player_flags{};

        int8_t m_gold = 0;

        player(game *game, int id) : m_game(game), id(id) {}

        void equip_card(card *card);
        card *find_equipped_card(card *card);

        void enable_equip(card *target_card);
        void disable_equip(card *target_card);

        card *random_hand_card();

        void add_cubes(card *target, int ncubes);
        void pay_cubes(card *target, int ncubes);
        void move_cubes(card *origin, card *target, int ncubes);
        void drop_all_cubes(card *target);

        bool can_escape(player *origin, card *origin_card, effect_flags flags) const;
        
        void add_to_hand(card *card);
        void add_to_hand_phase_one(card *card);
        void draw_card(int ncards = 1, card *origin_card = nullptr);

        void discard_card(card *target);
        void steal_card(card *target);

        int get_initial_cards();

        int max_cards_end_of_turn();

        int get_num_checks();
        int get_bangs_played();
        int get_cards_to_draw();

        bool is_ghost() const {
            return check_player_flags(player_flags::ghost)
                || check_player_flags(player_flags::temp_ghost);
        }

        bool alive() const {
            return !check_player_flags(player_flags::dead) || is_ghost();
        }

        void damage(card *origin_card, player *source, int value, effect_flags flags = {});

        void heal(int value);
        void set_hp(int value, bool instant = false);

        void add_gold(int amount);

        bool immune_to(card *origin_card, player *origin, effect_flags flags) const;
        
        void discard_all(bool death);

        bool only_black_cards_equipped() const;

        void next_predraw_check();

        void set_role(player_role role);
        void reset_max_hp();

        void set_last_played_card(card *c);

        std::vector<player *> make_equip_set(card *origin_card);
        std::vector<player *> make_player_target_set(card *origin_card, const effect_holder &effect);
        std::vector<card *> make_card_target_set(card *origin_card, const effect_holder &effect);

        bool is_possible_to_play(card *c, bool is_response = false);

        bool is_bangcard(card *origin_card);

        void draw_from_deck();

        void start_of_turn();
        void request_drawing();
        
        void pass_turn();
        void skip_turn();

        card_sign get_card_sign(card *target_card);

        void send_player_status();
        bool add_player_flags(player_flags flags);
        bool remove_player_flags(player_flags flags);
        bool check_player_flags(player_flags flags) const;

        int count_cubes() const;

        void untap_inactive_cards();
        void remove_extra_characters();

        void play_card_action(card *origin_card);
        void log_played_card(card *origin_card, bool is_response);

        void prompt_then(game_string &&message, std::function<void()> &&args);

        game_string handle_action(enums::enum_tag_t<game_action_type::pick_card>, card *target_card);
        game_string handle_action(enums::enum_tag_t<game_action_type::play_card>, const play_card_args &args);
        game_string handle_action(enums::enum_tag_t<game_action_type::prompt_respond>, bool response);
        game_string handle_action(enums::enum_tag_t<game_action_type::request_confirm>);
    };

}

#endif