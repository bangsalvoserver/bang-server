#include "rum.h"

#include "game/game.h"
#include "game/draw_check_handler.h"

namespace banggame {

    struct request_rum : request_base, draw_check_handler {
        request_rum(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_update() override {
            if (!live) {
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

            if (!target->m_game->call_event<event_type::on_draw_check_select>(target, shared_from_this(), false)) {
                resolve();
            }
        }

        std::vector<card *> get_drawn_cards() const override {
            return target->m_game->m_selection;
        }

        card *get_drawing_card() const override {
            return origin_card;
        }

        void restart() override {
            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
            }
            start();
        }

        int count_suits() const {
            std::vector<card_suit> suits;
            for (card *c : target->m_game->m_selection) {
                suits.push_back(target->m_game->get_card_sign(c).suit);
            }
            std::sort(suits.begin(), suits.end());
            return int(std::unique(suits.begin(), suits.end()) - suits.begin());
        }

        bool bot_check_redraw(card *target_card, player *owner) const override {
            return false;
        }

        void resolve() override {
            target->m_game->pop_request();
            int heal = count_suits();

            while (!target->m_game->m_selection.empty()) {
                card *drawn_card = target->m_game->m_selection.front();
                target->m_game->call_event<event_type::on_draw_check_resolve>(target, drawn_card);
                if (drawn_card->pocket == pocket_type::selection) {
                    target->m_game->move_card(drawn_card, pocket_type::discard_pile);
                }
            }
            target->heal(heal);
        }
    };

    bool effect_rum::on_check_target(card *origin_card, player *origin) {
        return origin->m_max_hp - origin->m_hp >= 2;
    }

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