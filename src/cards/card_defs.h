#ifndef __CARD_DEFS_H__
#define __CARD_DEFS_H__

#include "game_string.h"

#include "utils/tagged_variant.h"
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
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_2)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_9);
        }

        bool is_ten_to_ace() const {
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_10)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_A);
        }

        bool is_jack_to_ace() const {
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_J)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_A);
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
        cube,
        fame1,
        fame2,
        fame3,
        fame4,
        fame5,
        fame6,
        fame7,
        fame8
    };

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

    using cubes_and_players = std::pair<card_list, player_list>;

    using play_card_target = utils::tagged_variant<
        utils::tag<"none">,
        utils::tag<"player", player_ptr>,
        utils::tag<"conditional_player", nullable_player>,
        utils::tag<"adjacent_players", player_list>,
        utils::tag<"player_per_cube", cubes_and_players>,
        utils::tag<"random_if_hand_card", card_ptr>,
        utils::tag<"card", card_ptr>,
        utils::tag<"extra_card", nullable_card>,
        utils::tag<"players">,
        utils::tag<"cards", card_list>,
        utils::tag<"max_cards", card_list>,
        utils::tag<"card_per_player", card_list>,
        utils::tag<"cube_slot", card_ptr>,
        utils::tag<"move_cube_slot", card_list>,
        utils::tag<"select_cubes", card_list>,
        utils::tag<"select_cubes_optional", card_list>,
        utils::tag<"select_cubes_repeat", card_list>,
        utils::tag<"self_cubes">
    >;

    using target_type = utils::tagged_variant_index<play_card_target>;
    #define TARGET_TYPE(name) target_type{utils::tag<#name>{}}

    using target_list = std::vector<play_card_target>;

    struct effect_holder {
        target_type target;
        enums::bitset<target_player_filter> player_filter;
        enums::bitset<target_card_filter> card_filter;
        short target_value;
        const effect_vtable *type;
        const void *effect_value;

        explicit operator bool () const {
            return type != nullptr;
        }

        bool can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) const;

        void on_play(card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) const;
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

    struct mth_holder {
        const mth_vtable *type;
        const void *effect_value;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
    };

    using effect_list = std::span<const effect_holder>;
    using equip_list = std::span<const equip_holder>;
    using tag_map = utils::static_map_view<tag_type, short>;

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
        using cubes_max_pair = std::pair<card_list, int>;
        std::unordered_map<const_card_ptr, cubes_max_pair> m_value;

    public:
        void insert(const_card_ptr origin_card, card_list cubes, int max) {
            assert(max != 0);
            m_value.emplace(origin_card, std::pair{std::move(cubes), max});
        }

        int count(const_card_ptr origin_card) const {
            auto it = m_value.find(origin_card);
            if (it == m_value.end()) return 0;

            const auto &[cubes, max] = it->second;
            return cubes.size() / max;
        }

        auto all_cubes() const {
            return m_value | rv::values | rv::for_each(&cubes_max_pair::first);
        }
    };

    struct effect_context_base {
        nullable_card playing_card;
        nullable_card repeat_card;
        nullable_card card_choice;
        int8_t train_advance;
        bool ignore_distances;
    };

    struct effect_context : effect_context_base {
        player_list selected_players;
        card_list selected_cards;
        selected_cubes_count selected_cubes;
        nullable_player skipped_player;
        nullable_card traincost;
        nullable_card target_card;
        int8_t discount;
        bool disable_banglimit;
        bool disable_bang_checks;
        bool temp_missable;
    };

}

#endif