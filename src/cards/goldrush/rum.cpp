#include "rum.h"

#include "game/game.h"
#include "game/draw_check_handler.h"

namespace banggame {

    struct request_rum : request_base, draw_check_handler {
        request_rum(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_update() override {
            if (state == request_state::pending) {
                start();
            }
        }

        void start() {
            int num_cards = 3 + target->get_num_checks();
            for (int i=0; i < num_cards; ++i) {
                card *drawn_card = target->m_game->top_of_deck();
                target->m_game->add_log("LOG_REVEALED_CARD", target, drawn_card);
                target->m_game->move_card(drawn_card, pocket_type::selection);
            }

            if (target->m_game->call_event<event_type::on_draw_check_select>(target, origin_card, nullptr, true)) {
                resolve(nullptr);
            }
        }

        void restart() override {
            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
            }
            start();
        }

        bool check(card *) const override {
            return false;
        }

        void resolve(card *) override {
            target->m_game->pop_request();
            std::vector<card_suit> suits;

            while (!target->m_game->m_selection.empty()) {
                card *drawn_card = target->m_game->m_selection.front();
                suits.push_back(target->m_game->get_card_sign(drawn_card).suit);
                target->m_game->call_event<event_type::on_draw_check_resolve>(target, drawn_card);
                if (drawn_card->pocket == pocket_type::selection) {
                    target->m_game->move_card(drawn_card, pocket_type::discard_pile);
                }
            }
            std::sort(suits.begin(), suits.end());
            target->heal(int(std::unique(suits.begin(), suits.end()) - suits.begin()));
        }
    };

    game_string effect_rum::on_prompt(card *origin_card, player *origin) {
        if (origin->m_hp == origin->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_rum::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_rum>(origin_card, origin);
    }

}