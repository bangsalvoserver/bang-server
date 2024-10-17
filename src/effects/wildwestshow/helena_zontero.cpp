#include "helena_zontero.h"

#include "game/game.h"

#include "cards/game_enums.h"
#include "effects/base/draw_check.h"

namespace banggame {

    struct request_helena_zontero_check : request_base, draw_check_handler {
        request_helena_zontero_check(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, origin, nullptr) {}

        card_ptr drawn_card = nullptr;

        void on_update() override {
            if (!live) {
                origin_card->flash_card();
                restart();
            }
        }

        void restart() override {
            drawn_card = origin->m_game->top_of_deck();
            drawn_card->move_to(pocket_type::discard_pile);

            origin->m_game->add_log("LOG_CHECK_DREW_CARD", origin_card, static_cast<player_ptr>(nullptr), drawn_card);
            bool handled = false;
            origin->m_game->call_event(event_type::on_draw_check_select{ nullptr, shared_from_this(), handled });
            if (!handled) {
                resolve();
            }
        }

        card_list get_drawn_cards() const override {
            return {drawn_card};
        }

        card_ptr get_drawing_card() const override {
            return origin_card;
        }

        bool is_lucky() const {
            return drawn_card->get_modified_sign().is_red();
        }

        prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const override {
            if (owner->is_bot() && !is_lucky()) {
                return "BOT_BAD_PLAY";
            }
            return {};
        }
        
        void resolve() override {
            origin->m_game->pop_request();
            if (is_lucky()) {
                auto alive_players = origin->m_game->m_players
                    | rv::filter([](player_ptr p) {
                        return p->alive() && p->m_role != player_role::sheriff;
                    });
                
                auto roles = alive_players | rv::transform(&player::m_role) | rn::to_vector;
                rn::shuffle(roles, origin->m_game->rng);
                
                for (player_ptr p : alive_players) {
                    p->set_role(player_role::unknown, false);
                    p->remove_player_flags(player_flag::role_revealed);
                }

                for (auto [p, role] : rv::zip(alive_players, roles)) {
                    p->set_role(role, false);
                }
            }
        }
    };

    void equip_helena_zontero::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->queue_request<request_helena_zontero_check>(target_card, origin);
    }
}