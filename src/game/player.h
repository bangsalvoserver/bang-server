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
        card(int id, const card_data &data): card_data(data), id(id) {}
        
        int id;

        player *owner = nullptr;
        pocket_type pocket = pocket_type::none;
        card_visibility visibility = card_visibility::hidden;
        
        bool inactive = false;
        int8_t num_cubes = 0;

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

    inline card_backface::card_backface(card *c): id(c->id), deck(c->deck) {}

    using draw_check_condition = std::function<bool(card_sign)>;
    using draw_check_function = std::function<void(bool)>;
    
    struct card_pocket_pair {
        card *origin_card;
        pocket_type pocket;
    };

    struct effect_context_deleter {
        void operator ()(effect_context *ctx) const noexcept;
    };

    using effect_context_ptr = std::unique_ptr<effect_context, effect_context_deleter>;
    using shared_effect_context = std::shared_ptr<effect_context>;

    shared_effect_context make_shared_effect_context(effect_context &&ctx);

    struct played_card_history {
        card_pocket_pair origin_card;
        std::vector<card_pocket_pair> modifiers;
        effect_context_ptr context;

        played_card_history(card *origin_card, const modifier_list &modifiers, const effect_context &context);
        played_card_history(played_card_history &&) noexcept = default;
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

        int8_t m_range_mod = 0;
        int8_t m_weapon_range = 1;
        int8_t m_distance_mod = 0;

        int8_t m_hp = 0;
        int8_t m_max_hp = 0;
        
        int8_t m_num_drawn_cards = 0;
        
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

        void add_cubes(card *target, int ncubes);
        void pay_cubes(card *target, int ncubes);
        void move_cubes(card *origin, card *target, int ncubes);
        void drop_all_cubes(card *target);

        int can_escape(player *origin, card *origin_card, effect_flags flags) const;
        
        void add_to_hand(card *card);
        void add_to_hand_phase_one(card *card);
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
        int get_cards_to_draw();

        bool is_bot() const;
        bool is_ghost() const;
        bool alive() const;

        void damage(card *origin_card, player *source, int value, effect_flags flags = {});

        void heal(int value);
        void set_hp(int value, bool instant = false);

        void add_gold(int amount);

        bool immune_to(card *origin_card, player *origin, effect_flags flags) const;

        bool only_black_cards_equipped() const {
            return empty_hand() && std::ranges::all_of(m_table, &card::is_black);
        }

        bool empty_hand() const {
            return m_hand.empty();
        }

        card *first_character() const {
            return m_characters.front();
        }

        void set_role(player_role role, bool instant = true);
        void reset_max_hp();

        void start_of_turn();
        void request_drawing();
        
        void pass_turn();
        void skip_turn();

        void send_player_status();
        bool add_player_flags(player_flags flags);
        bool remove_player_flags(player_flags flags);
        bool check_player_flags(player_flags flags) const;

        int count_cubes() const;

        auto cube_slots() const {
            return ranges::views::concat(
                m_table | ranges::views::filter(&card::is_orange),
                ranges::views::single(first_character())
            );
        }

        void untap_inactive_cards();
        void remove_extra_characters();

        void handle_game_action(const game_action &action);
    };

}

#endif