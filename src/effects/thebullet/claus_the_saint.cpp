#include "claus_the_saint.h"

#include "effects/base/draw.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    struct request_claus_the_saint : request_base, interface_target_set {
        request_claus_the_saint(card *origin_card, player *target, shared_request_draw &&req_draw)
            : request_base(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        std::vector<player *> selected_targets;

        shared_request_draw req_draw;

        void on_update() override {
            if (!live) {
                int ncards = target->m_game->num_alive() + req_draw->num_cards_to_draw - 1;
                for (int i=0; i<ncards; ++i) {
                    req_draw->phase_one_drawn_card()->move_to(pocket_type::selection, target);
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

        bool in_target_set(const player *target_player) const override {
            return target_player != target && !rn::contains(selected_targets, target_player);
        }
    };
    
    void equip_claus_the_saint::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin, shared_request_draw req_draw, bool &handled) {
            if (!handled && origin == target) {
                target->m_game->queue_request<request_claus_the_saint>(target_card, target, std::move(req_draw));
                handled = true;
            }
        });
    }

    bool effect_claus_the_saint::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_claus_the_saint>(origin) != nullptr;
    }

    void handler_claus_the_saint::on_play(card *origin_card, player *origin, card *target_card, player *target_player) {
        auto req = origin->m_game->top_request<request_claus_the_saint>(origin);
        req->selected_targets.push_back(target_player);
        
        if (!origin->m_game->check_flags(game_flags::hands_shown)) {
            origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_GIFTED_CARD", origin, target_player, target_card);
            origin->m_game->add_log(update_target::excludes(origin, target_player), "LOG_GIFTED_A_CARD", origin, target_player);
        } else {
            origin->m_game->add_log("LOG_GIFTED_CARD", origin, target_player, target_card);
        }
        target_player->add_to_hand(target_card);

        if (origin->m_game->m_selection.size() <= req->req_draw->num_cards_to_draw) {
            while (!origin->m_game->m_selection.empty()) {
                req->req_draw->add_to_hand_phase_one(origin->m_game->m_selection.front());
            }
            
            origin->m_game->pop_request();
        }
    }
}