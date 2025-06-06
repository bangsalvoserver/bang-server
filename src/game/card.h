#ifndef __BANGGAME_CARD_H__
#define __BANGGAME_CARD_H__

#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#include "game_update.h"
#include "game_net.h"
#include "player.h"

namespace banggame {

    enum class card_visibility : uint8_t {
        hidden,
        shown,
        show_owner
    };
    
    static constexpr int max_cubes = 4;
    
    struct card : card_data {
        card(game_ptr game, int id, const card_data &data)
            : card_data(data), m_game(game), order(id), id(id) {}
        
        const int order;
        int id;

        game_ptr m_game = nullptr;
        player_ptr owner = nullptr;
        pocket_type pocket = pocket_type::none;
        update_target visibility;
        
        bool inactive = false;
        token_map tokens;

        bool is_equip_card() const;
        bool is_bang_card(const_player_ptr origin) const;

        card_visibility get_visibility() const;
        void set_visibility(update_target visibility, bool instant = false);
        void set_visibility(card_visibility visibility, player_ptr owner = nullptr, bool instant = false);
        void move_to(pocket_type pocket, player_ptr owner = nullptr, card_visibility visibility = card_visibility::show_owner, bool instant = false, pocket_position position = pocket_position::end);
        void set_inactive(bool inactive);

        void flash_card();
        void add_short_pause();

        int num_tokens(card_token_type token_type) const {
            return tokens[token_type];
        }

        int num_cubes() const {
            return num_tokens(card_token_type::cube);
        }

        void add_cubes(int ncubes);
        void move_cubes(card_ptr target, int ncubes, bool instant = false);

        void drop_all_cubes();
        void drop_all_fame();
    };

    inline auto cube_slots(const_player_ptr target) {
        return rv::concat(
            target->m_characters | rv::take(1),
            target->m_table | rv::filter(&card::is_orange)
        );
    }

    inline int count_cubes(const_player_ptr target) {
        return rn::accumulate(cube_slots(target)
            | rv::transform(&card::num_cubes), 0);
    }

}

#endif