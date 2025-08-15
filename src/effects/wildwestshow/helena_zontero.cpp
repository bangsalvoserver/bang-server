#include "helena_zontero.h"

#include "cards/game_enums.h"

#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    struct request_helena_zontero_check : request_base, draw_check_handler, std::enable_shared_from_this<request_helena_zontero_check> {
        request_helena_zontero_check(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, origin, nullptr) {}

        card_ptr drawn_card = nullptr;

        void on_update() override {
            if (update_count == 0) {
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

        bool is_positive() const {
            return get_modified_sign(drawn_card).is_red();
        }

        prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const override {
            if (owner->is_bot() && !is_positive()) {
                return "BOT_DONT_REDRAW_HELENA_ZONTERO";
            }
            return {};
        }
        
        void resolve() override {
            origin->m_game->pop_request();
            if (is_positive()) {
                auto alive_players = origin->m_game->m_players
                    | rv::filter([](player_ptr p) {
                        return p->alive() && p->m_role != player_role::sheriff;
                    });
                
                auto roles = alive_players | rv::transform(&player::get_base_role) | rn::to<std::vector>();
                rn::shuffle(roles, origin->m_game->rng);
                
                for (player_ptr p : alive_players) {
                    p->set_role(player_role::unknown);
                    p->remove_player_flags(player_flag::role_revealed);
                }

                for (auto [p, role] : rv::zip(alive_players, roles)) {
                    p->set_role(role);
                }
            }
        }
    };

    void equip_helena_zontero::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->queue_request<request_helena_zontero_check>(target_card, origin);
    }
}