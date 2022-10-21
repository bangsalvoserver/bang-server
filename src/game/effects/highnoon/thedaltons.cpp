#include "thedaltons.h"

#include "../../game.h"

namespace banggame {

    struct request_thedaltons : request_base {
        request_thedaltons(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, effect_flags::auto_pick) {}

        bool can_pick(pocket_type pocket, player *target_player, card *target_card) const override {
            return pocket == pocket_type::player_table
                && target == target_player
                && target_card->color == card_color_type::blue;
        }

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
            target->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_THEDALTONS", origin_card};
            } else {
                return {"STATUS_THEDALTONS_OTHER", target, origin_card};
            }
        }
    };

    void effect_thedaltons::on_enable(card *target_card, player *target) {
        for (player &p : range_all_players(target)) {
            if (std::ranges::find(p.m_table, card_color_type::blue, &card::color) != p.m_table.end()) {
                p.m_game->queue_request<request_thedaltons>(target_card, &p);
            }
        }
    }
}