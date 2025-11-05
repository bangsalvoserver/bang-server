#ifndef __CARD_DEFS_H__
#define __CARD_DEFS_H__

#include "game_string.h"

#include "utils/enum_bitset.h"
#include "utils/enum_map.h"

namespace banggame {

    enum class card_suit {
        none,
        hearts,
        diamonds,
        clubs,
        spades,
    };

    enum class card_rank {
        none,
        rank_2,
        rank_3,
        rank_4,
        rank_5,
        rank_6,
        rank_7,
        rank_8,
        rank_9,
        rank_10,
        rank_J,
        rank_Q,
        rank_K,
        rank_A,
    };

    struct card_sign {
        card_suit suit;
        card_rank rank;

        bool is_hearts() const      { return suit == card_suit::hearts; }
        bool is_diamonds() const    { return suit == card_suit::diamonds; }
        bool is_red() const         { return is_hearts() || is_diamonds(); }
        bool is_clubs() const       { return suit == card_suit::clubs; }
        bool is_spades() const      { return suit == card_suit::spades; }
        bool is_black() const       { return is_clubs() || is_spades(); }

        bool is_two_to_nine() const {
            return enums::is_between(rank, card_rank::rank_2, card_rank::rank_9);
        }

        bool is_ten_to_ace() const {
            return enums::is_between(rank, card_rank::rank_10, card_rank::rank_A);
        }

        bool is_jack_to_ace() const {
            return enums::is_between(rank, card_rank::rank_J, card_rank::rank_A);
        }

        explicit operator bool () const {
            return suit != card_suit::none && rank != card_rank::none;
        }
    };

    enum class card_color_type {
        none,
        brown,
        blue,
        green,
        black,
        orange,
        train,
    };

    enum class card_token_type {
        fame1,
        fame2,
        fame3,
        fame4,
        fame5,
        fame6,
        fame7,
        fame8,
        cube,
        gold
    };

    constexpr bool is_fame_token(card_token_type token) {
        return enums::is_between(token, card_token_type::fame1, card_token_type::fame8);
    }

    using token_map = enums::enum_map<card_token_type, uint8_t>;

    enum class player_role {
        unknown,
        sheriff,
        deputy,
        outlaw,
        renegade,
        deputy_3p,
        outlaw_3p,
        renegade_3p,
        shadow_deputy,
        shadow_outlaw
    };
    
    class play_card_target {
    private:
        std::any m_value;
    
    public:
        template<typename T>
        play_card_target(T &&value) : m_value{std::forward<T>(value)} {}
    
        template<typename T>
        const T &get() const {
            return std::any_cast<const T &>(m_value);
        }
    };

    using target_list = std::vector<play_card_target>;

    struct effect_holder {
        const effect_vtable *type;
        const void *effect_value;
        const targeting_vtable *target;
        const void *target_value;

        explicit operator bool () const {
            return type != nullptr;
        }

        bool can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;

        std::generator<play_card_target> possible_targets(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        play_card_target random_target(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, const play_card_target &target, const effect_context &ctx) const;
        game_string get_error_mth(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
        
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const play_card_target &target, const effect_context &ctx) const;
        prompt_string on_prompt_mth(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, const play_card_target &target, effect_context &ctx) const;
        void add_context_mth(card_ptr origin_card, player_ptr origin, const target_list &targets, effect_context &ctx) const;

        void on_play(card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, const play_card_target &target, const effect_context &ctx) const;
        void on_play_mth(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
    };

    struct equip_holder {
        const equip_vtable *type;
        const void *effect_value;

        explicit operator bool () const {
            return type != nullptr;
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
        bool is_nodisable() const;
    };

    struct modifier_holder {
        const modifier_vtable *type;
        const void *effect_value;

        explicit operator bool () const {
            return type != nullptr;
        }

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
    };

    using effect_list = std::span<const effect_holder>;
    using equip_list = std::span<const equip_holder>;
    using tag_int = int8_t;
    using tag_map = utils::static_map_view<tag_type, tag_int>;

    enum class card_deck_type {
        none,
        main_deck,
        character,
        role,
        goldrush,
        highnoon,
        fistfulofcards,
        wildwestshow,
        station,
        locomotive,
        train,
        legends,
        feats
    };

    enum class pocket_type {
        none,
        player_hand,
        player_table,
        player_character,
        main_deck,
        discard_pile,
        selection,
        shop_deck,
        shop_selection,
        hidden_deck,
        scenario_deck,
        scenario_card,
        wws_scenario_deck,
        wws_scenario_card,
        button_row,
        stations,
        train_deck,
        train,
        feats_deck,
        feats_discard,
        feats
    };

    class selected_cubes_count {
    private:
        struct selected_cubes_entry {
            const_card_ptr origin_card;
            card_list cubes;
            int ncubes;
        };

        std::vector<selected_cubes_entry> m_value;

    public:
        void insert(const_card_ptr origin_card, card_list cubes, int ncubes) {
            m_value.emplace_back(origin_card, std::move(cubes), ncubes);
        }

        int count(const_card_ptr origin_card) const {
            int result = 0;
            for (const auto &entry : m_value) {
                if (entry.origin_card == origin_card && entry.ncubes != 0) {
                    result += entry.cubes.size() / entry.ncubes;
                }
            }
            return result;
        }

        auto all_cubes() const {
            return m_value | rv::for_each(&selected_cubes_entry::cubes);
        }
    };

    struct effect_context_base {
        struct remove_defaults{};
        
        card_ptr playing_card;
        card_ptr repeat_card;
        card_ptr card_choice;
        int8_t train_advance;
        bool ignore_distances;
    };

    struct effect_context : effect_context_base {
        player_list selected_players;
        card_list selected_cards;
        selected_cubes_count selected_cubes;
        player_ptr skipped_player;
        card_ptr traincost;
        card_ptr target_card;
        int8_t discount;
        bool disable_banglimit;
        bool disable_bang_checks;
        bool temp_missable;
    };

}

#endif