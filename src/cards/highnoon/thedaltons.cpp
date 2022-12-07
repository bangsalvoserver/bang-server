#include "thedaltons.h"

#include "game/game.h"

namespace banggame {

    struct request_thedaltons : request_base {
        request_thedaltons(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        bool auto_resolve() override {
            return auto_pick();
        }

        bool can_pick(card *target_card) const override {
            return target_card->pocket == pocket_type::player_table
                && target_card->owner == target
                && target_card->color == card_color_type::blue;
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_THEDALTONS", origin_card};
            } else {
                return {"STATUS_THEDALTONS_OTHER", target, origin_card};
            }
        }
    };

    void equip_thedaltons::on_enable(card *target_card, player *target) {
        for (player *p : range_all_players(target)) {
            if (std::ranges::find(p->m_table, card_color_type::blue, &card::color) != p->m_table.end()) {
                p->m_game->queue_request<request_thedaltons>(target_card, p);
            }
        }
    }
}