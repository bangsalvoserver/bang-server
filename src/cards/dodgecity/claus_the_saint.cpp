#include "claus_the_saint.h"

#include "cards/base/draw.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    struct request_claus_the_saint : request_base {
        request_claus_the_saint(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond) {}

        std::vector<player *> targets;

        void on_update() override {
            if (state == request_state::pending) {
                int ncards = target->m_game->num_alive() + target->get_cards_to_draw() - 1;
                for (int i=0; i<ncards; ++i) {
                    target->m_game->move_card(target->m_game->phase_one_drawn_card(), pocket_type::selection, target);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_CLAUS_THE_SAINT", origin_card};
            } else {
                return {"STATUS_CLAUS_THE_SAINT_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_claus_the_saint::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin) {
            if (target->m_game->top_request<request_draw>(target) && origin == target) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    target->m_game->queue_request<request_claus_the_saint>(target_card, target);
                });
            }
        });
    }

    game_string handler_claus_the_saint::get_error(card *origin_card, player *origin, card *target_card, player *target_player) {
        if (auto req = origin->m_game->top_request<request_claus_the_saint>(origin)) {
            if (ranges::contains(req->targets, target_player)) {
                return "ERROR_TARGET_NOT_UNIQUE";
            } else {
                return {};
            }
        } else {
            return "ERROR_INVALID_RESPONSE";
        }
    }

    void handler_claus_the_saint::on_play(card *origin_card, player *origin, card *target_card, player *target_player) {
        origin->m_game->top_request<request_claus_the_saint>(origin)->targets.push_back(target_player);
        
        if (!origin->m_game->check_flags(game_flags::hands_shown)) {
            origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_GIFTED_CARD", origin, target_player, target_card);
            origin->m_game->add_log(update_target::excludes(origin, target_player), "LOG_GIFTED_A_CARD", origin, target_player);
        } else {
            origin->m_game->add_log("LOG_GIFTED_CARD", origin, target_player, target_card);
        }
        target_player->add_to_hand(target_card);

        if (origin->m_game->m_selection.size() <= origin->get_cards_to_draw()) {
            while (!origin->m_game->m_selection.empty()) {
                origin->add_to_hand_phase_one(origin->m_game->m_selection.front());
            }
            
            origin->m_game->pop_request();
        }
    }
}