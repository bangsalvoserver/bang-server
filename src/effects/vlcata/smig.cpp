#include "smig.h"

#include "cards/filter_enums.h"
#include "effects/base/bang.h"
#include "effects/base/resolve.h"
#include "effects/base/steal_destroy.h"

#include "game/game_table.h"

namespace banggame {

    struct request_smig_destroy : request_resolvable, interface_missable {
        request_smig_destroy(card_ptr origin_card, player_ptr origin, card_ptr target_card)
            : request_resolvable(origin_card, origin, target_card->owner)
            , target_card(target_card) {}

        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return {target_card};
        }

        void on_update() override {
            if (target->empty_hand()) {
                auto_resolve();
            }
        }

        void on_resolve() override {
            pop_request();
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
        }

        bool can_miss(card_ptr c) const override {
            return c->has_tag(tag_type::missedcard) && interface_missable::can_miss(c);
        }

        void on_miss(card_ptr c, effect_flags missed_flags = {}) override {
            pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_RICOCHET", origin_card, target_card};
            } else {
                return {"STATUS_RICOCHET_OTHER", target, origin_card, target_card};
            }
        }
    };

    void effect_smig_destroy::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->queue_request<request_smig_destroy>(origin_card, origin, target_card);
    }

}
