#include "helena_zontero.h"

#include "game/game.h"

#include "cards/game_enums.h"
#include "effects/base/draw_check.h"

namespace banggame {

    struct request_helena_zontero_check : request_base, draw_check_handler {
        request_helena_zontero_check(card *origin_card, player *origin)
            : request_base(origin_card, origin, nullptr) {}

        card *drawn_card = nullptr;

        void on_update() override {
            if (!live) {
                origin->m_game->flash_card(origin_card);
                restart();
            }
        }

        void restart() override {
            drawn_card = origin->m_game->top_of_deck();
            origin->m_game->move_card(drawn_card, pocket_type::discard_pile);

            origin->m_game->add_log("LOG_CHECK_DREW_CARD", origin_card, nullptr, drawn_card);
            bool handled = false;
            origin->m_game->call_event(event_type::on_draw_check_select{ nullptr, shared_from_this(), handled });
            if (!handled) {
                resolve();
            }
        }

        std::vector<card *> get_drawn_cards() const override {
            return {drawn_card};
        }

        card *get_drawing_card() const override {
            return origin_card;
        }

        bool is_lucky() const {
            return origin->m_game->get_card_sign(drawn_card).is_red();
        }

        bool bot_check_redraw(card *target_card, player *owner) const override {
            return !is_lucky();
        }
        
        void resolve() override {
            origin->m_game->pop_request();
            if (is_lucky()) {
                auto alive_players = origin->m_game->m_players
                    | rv::filter([](player *p) {
                        return p->alive() && p->m_role != player_role::sheriff;
                    });
                
                auto roles = alive_players | rv::transform(&player::m_role) | rn::to_vector;
                rn::shuffle(roles, origin->m_game->rng);
                
                for (player *p : alive_players) {
                    p->set_role(player_role::unknown, false);
                    p->remove_player_flags(player_flags::role_revealed);
                }

                for (auto [p, role] : rv::zip(alive_players, roles)) {
                    p->set_role(role, false);
                }
            }
        }
    };

    void equip_helena_zontero::on_enable(card *target_card, player *origin) {
        origin->m_game->queue_request<request_helena_zontero_check>(target_card, origin);
    }
}