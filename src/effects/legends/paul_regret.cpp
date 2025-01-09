#include "paul_regret.h"

#include "effects/base/bang.h"
#include "effects/base/requests.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void equip_paul_regret_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_bang_target>(origin_card, [=](card_ptr e_origin_card, player_ptr e_origin, player_ptr e_target, effect_flags flags, game_string &out_error) {
            if (e_origin_card && e_origin != e_target && e_target == origin && e_origin->m_hand.size() == 1 && flags.check(effect_flag::is_bang)) {
                out_error = {"ERROR_CANNOT_TARGET_PLAYER", origin_card, origin, e_origin_card};
            }
        });

        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr target, shared_request_bang req) {
            if (origin != target && req->target == origin) {
                origin->m_game->queue_request<request_discard>(origin_card, origin, target, effect_flags{}, 210);
            }
        });
    }
}