#ifndef __BANGGAME_CARD_H__
#define __BANGGAME_CARD_H__

#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#include "game_update.h"

namespace banggame {

    enum class card_visibility : uint8_t {
        hidden,
        shown,
        show_owner
    };
    
    struct card : card_data {
        card(game *game, int id, const card_data &data)
            : card_data(data), m_game(game), order(id), id(id) {}
        
        const int order;
        int id;

        game *m_game = nullptr;
        player_ptr owner = nullptr;
        pocket_type pocket = pocket_type::none;
        card_visibility visibility = card_visibility::hidden;
        
        bool inactive = false;
        token_map tokens;

        bool is_equip_card() const;
        bool is_bang_card(const_player_ptr origin) const;
        int get_card_cost(const effect_context &ctx) const;
        card_sign get_modified_sign() const;

        void set_visibility(card_visibility visibility, player_ptr owner = nullptr, bool instant = false);
        void move_to(pocket_type pocket, player_ptr owner = nullptr, card_visibility visibility = card_visibility::show_owner, bool instant = false, bool front = false);
        void set_inactive(bool inactive);

        void flash_card();
        void add_short_pause();
        
        static int get_max_tokens(card_token_type token_type);

        int num_tokens(card_token_type token_type) const;
        void add_tokens(card_token_type token_type, int num_tokens);
        void move_tokens(card_token_type token_type, card_ptr target, int num_tokens, bool instant = false);
        void drop_all_tokens();

        int num_cubes() const {
            return num_tokens(card_token_type::cube);
        }

        void add_cubes(int ncubes) {
            add_tokens(card_token_type::cube, ncubes);
        }

        void move_cubes(card_ptr target, int ncubes, bool instant = false) {
            move_tokens(card_token_type::cube, target, ncubes, instant);
        }
    };
    
    struct card_pocket_pair {
        card_ptr origin_card;
        pocket_type pocket;
    };

    struct played_card_history {
        card_pocket_pair origin_card;
        std::vector<card_pocket_pair> modifiers;
        bool is_response;
        effect_context context;
    };

}

#endif