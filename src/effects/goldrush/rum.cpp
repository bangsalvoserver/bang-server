#include "rum.h"

#include "effects/base/draw_check.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_rum : request_base, draw_check_handler, std::enable_shared_from_this<request_rum> {
        request_rum(card_ptr origin_card, player_ptr target)
            : request_base(origin_card, nullptr, target) {}

        void on_update() override {
            if (update_count == 0) {
                start();
            }
        }

        void start() {
            int num_cards = 3 + target->get_num_checks();
            for (int i=0; i < num_cards; ++i) {
                card_ptr drawn_card = target->m_game->top_of_deck();
                target->m_game->add_log("LOG_REVEALED_CARD", target, drawn_card);
                drawn_card->move_to(pocket_type::selection);
            }

            bool handled = false;
            target->m_game->call_event(event_type::on_draw_check_select{ target, shared_from_this(), handled });
            if (!handled) {
                resolve();
            }
        }

        card_list get_drawn_cards() const override {
            return target->m_game->m_selection;
        }

        card_ptr get_drawing_card() const override {
            return origin_card;
        }

        void restart() override {
            while (!target->m_game->m_selection.empty()) {
                target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
            }
            start();
        }

        int count_suits() const {
            std::vector<card_suit> suits;
            for (card_ptr c : target->m_game->m_selection) {
                suits.push_back(get_modified_sign(c).suit);
            }
            std::sort(suits.begin(), suits.end());
            return int(std::unique(suits.begin(), suits.end()) - suits.begin());
        }

        prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const override {
            if (owner->is_bot()) {
                return "BOT_DONT_REDRAW_RUM";
            }
            return {};
        }

        void resolve() override {
            target->m_game->pop_request();
            int heal = count_suits();

            while (!target->m_game->m_selection.empty()) {
                card_ptr drawn_card = target->m_game->m_selection.front();
                target->m_game->call_event(event_type::on_draw_check_resolve{ origin_card, target, drawn_card, nullptr });
                if (drawn_card->pocket == pocket_type::selection) {
                    drawn_card->move_to(pocket_type::discard_pile);
                }
            }
            target->heal(heal);
        }
    };

    game_string effect_rum::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->is_bot() && origin->m_max_hp - origin->m_hp < 2) {
            return "BOT_WASTEFUL_RUM";
        }
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, origin));
        if (origin->m_hp == origin->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    void effect_rum::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_rum>(origin_card, origin);
    }

}