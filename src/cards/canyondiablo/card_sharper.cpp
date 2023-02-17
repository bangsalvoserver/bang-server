#include "card_sharper.h"

#include "game/game.h"
#include "game/filters.h"
#include "cards/base/steal_destroy.h"

namespace banggame {

    static void resolve_card_sharper(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        player *target = target_card->owner;
        origin->m_game->add_log("LOG_SWAP_CARDS", origin, target, chosen_card, target_card);

        target->disable_equip(target_card);
        target_card->on_unequip(target);
        target_card->on_equip(origin);
        origin->equip_card(target_card);
        if (chosen_card->owner == origin) {
            origin->disable_equip(chosen_card);
            chosen_card->on_unequip(origin);
        }
        chosen_card->on_equip(target);
        target->equip_card(chosen_card);
    }

    struct request_card_sharper : request_targeting {
        request_card_sharper(card *origin_card, player *origin, player *target, card *chosen_card, card *target_card)
            : request_targeting(origin_card, origin, target, target_card, effect_flags::escapable)
            , chosen_card(chosen_card) {}

        card *chosen_card;

        std::vector<card *> get_highlights() const override {
            return {target_card, chosen_card};
        }

        void on_resolve_target() override {
            resolve_card_sharper(origin_card, origin, chosen_card, target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CARD_SHARPER", origin_card, target_card, chosen_card};
            } else {
                return {"STATUS_CARD_SHARPER_OTHER", target, origin_card, target_card, chosen_card};
            }
        }
    };

    game_string handler_card_sharper::get_error(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        player *target = target_card->owner;
        MAYBE_RETURN(check_player_filter(origin, target_card->equip_target, origin));
        if (auto *c = origin->find_equipped_card(target_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        MAYBE_RETURN(check_player_filter(target, chosen_card->equip_target, target));
        if (auto *c = target->find_equipped_card(chosen_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        return {};
    }

    void handler_card_sharper::on_play(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        origin->m_game->queue_request<request_card_sharper>(origin_card, origin, target_card->owner, chosen_card, target_card);
    }
}