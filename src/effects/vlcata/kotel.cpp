#include "kotel.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"
#include "effects/base/bang.h"
#include "effects/greattrainrobbery/map.h"

#include "game/game_table.h"

namespace banggame {

    static bool has_weapon(player_ptr p) {
        for (card_ptr c : p->m_table) {
            if (c->has_tag(tag_type::weapon)) return true;
        }
        return false;
    }

    static bool has_mustang(player_ptr p) {
        for (card_ptr c : p->m_table) {
            if (c->name == "MUSTANG" || c->name == "HIDEOUT") return true;
        }
        return false;
    }

    static bool has_scope(player_ptr p) {
        for (card_ptr c : p->m_table) {
            if (c->name == "SCOPE" || c->name == "BINOCULAR") return true;
        }
        return false;
    }

    void equip_kotel::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_range_mod>({target_card, -10}, [target](const_player_ptr origin, range_mod_type type, int &value) {
            if (origin != target) return;
            if (type == range_mod_type::weapon_range && has_weapon(target)) {
                ++value;
            } else if (type == range_mod_type::distance_mod && has_mustang(target)) {
                ++value;
            } else if (type == range_mod_type::range_mod && has_scope(target)) {
                ++value;
            }
        });

        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [target](const_player_ptr origin, int &value, bool real_count) {
            if (origin == target && !real_count && has_weapon(target) && value > 0) {
                --value;
            }
        });

        target->m_game->add_listener<event_type::on_missed>(target_card, [target, target_card](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
            if (req->target == target && missed_card && missed_card->name == "BARREL") {
                target_card->flash_card();
                target->draw_card(2, target_card);
            }
        });

        target->m_game->add_listener<event_type::count_map_reveal>(target_card, [target](const_player_ptr origin, int &value) {
            if (origin == target) {
                ++value;
            }
        });
    }

}
