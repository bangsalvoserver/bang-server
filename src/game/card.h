#ifndef __BANGGAME_CARD_H__
#define __BANGGAME_CARD_H__

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
        card(game *game, int id, const card_data &data)
            : card_data(data), m_game(game), order(id), id(id) {}
        
        const int order;
        int id;

        game *m_game = nullptr;
        player_ptr owner = nullptr;
        pocket_type pocket = pocket_type::none;
        card_visibility visibility = card_visibility::hidden;
        
        bool inactive = false;
        int8_t num_cubes = 0;
        int8_t num_fame = 0;

        bool is_equip_card() const;
        bool is_bang_card(const_player_ptr origin) const;
        int get_card_cost(const effect_context &ctx) const;
        card_sign get_modified_sign() const;

        void set_visibility(card_visibility visibility, player_ptr owner = nullptr, bool instant = false);
        void move_to(pocket_type pocket, player_ptr owner = nullptr, card_visibility visibility = card_visibility::show_owner, bool instant = false, bool front = false);
        void set_inactive(bool inactive);

        void flash_card();
        void add_short_pause();

        void add_cubes(int ncubes);
        void move_cubes(card_ptr target, int ncubes, bool instant = false);
        void drop_cubes();

        void add_fame(int fame);
        void move_fame(card_ptr target, int fame, bool instant = false);
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