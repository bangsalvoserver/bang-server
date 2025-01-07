#include "annie_oakey.h"

#include "game/game.h"

#include "effects/base/draw.h"
#include "effects/base/resolve.h"

namespace banggame {

    struct request_annie_oakey : request_base, interface_resolvable {
        request_annie_oakey(card_ptr origin_card, player_ptr target, shared_request_draw &&req_draw)
            : request_base(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        shared_request_draw req_draw;

        void on_update() override {
            if (!live) {
                req_draw->cleanup_selection();
            }
            if (!target->alive() || req_draw->num_drawn_cards >= req_draw->num_cards_to_draw) {
                target->m_game->pop_request();
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            req_draw->add_to_hand_phase_one(req_draw->phase_one_drawn_card());
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_ANNIE_OAKEY", origin_card};
            } else {
                return {"STATUS_ANNIE_OAKEY_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_annie_oakey::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, shared_request_draw req_draw, bool &handled) {
            if (!handled && origin == target && req_draw->num_cards_to_draw < 3) {
                target->m_game->queue_request<request_annie_oakey>(target_card, target, std::move(req_draw));
                handled = true;
            }
        });
    }

    bool effect_annie_oakey::can_play(card_ptr origin_card, player_ptr target) {
        return target->m_game->top_request<request_annie_oakey>(target_is{target}) != nullptr;
    }

    static int get_ncards(int choice, card_sign sign) {
        static constexpr std::pair<bool (card_sign::*)() const, int> lookup[] = {
            {&card_sign::is_red, 1},
            {&card_sign::is_hearts, 2},
            {&card_sign::is_diamonds, 2},
            {&card_sign::is_black, 1},
            {&card_sign::is_clubs, 2},
            {&card_sign::is_spades, 2}
        };
        if (choice >= 1 && choice <= std::size(lookup)) {
            auto &[fn, ncards] = lookup[choice-1];
            return static_cast<int>(std::invoke(fn, sign)) * ncards;
        }
        throw game_error("Invalid choice");
    }

    void effect_annie_oakey::on_play(card_ptr origin_card, player_ptr target) {
        shared_request_draw req_draw = target->m_game->top_request<request_annie_oakey>()->req_draw;

        card_ptr drawn_card = req_draw->phase_one_drawn_card();
        drawn_card->set_visibility(card_visibility::shown);
        drawn_card->add_short_pause();

        req_draw->num_cards_to_draw += get_ncards(choice, drawn_card->sign);
        req_draw->add_to_hand_phase_one(drawn_card);
    }
}