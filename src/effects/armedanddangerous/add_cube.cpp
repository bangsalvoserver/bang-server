#include "add_cube.h"

#include "game/game.h"

#include "effects/base/pick.h"

namespace banggame {
    
    struct request_add_cube : request_picking {
        request_add_cube(card_ptr origin_card, player_ptr target, int ncubes = 1)
            : request_picking(origin_card, nullptr, target)
            , ncubes(ncubes) {}

        int ncubes = 1;

        void on_update() override {
            int nslots = 0;
            int ncards = 0;
            for (card_ptr c : target->cube_slots()) {
                ncards += c->num_cubes < max_cubes;
                nslots += max_cubes - c->num_cubes;
            }

            if (nslots <= ncubes || ncards <= 1) {
                target->m_game->pop_request();
                for (card_ptr c : target->cube_slots()) {
                    int cubes_to_add = std::min<int>(ncubes, max_cubes - c->num_cubes);
                    ncubes -= cubes_to_add;
                    c->add_cubes(cubes_to_add);
                }
            }
        }
        
        bool can_pick(const_card_ptr target_card) const override {
            return target_card->owner == target
                && (target_card->pocket == pocket_type::player_table && target_card->is_orange()
                || target_card->pocket == pocket_type::player_character && target_card == target->first_character())
                && target_card->num_cubes < max_cubes;
        }

        void on_pick(card_ptr target_card) override {
            if (--ncubes == 0) {
                target->m_game->pop_request();
            }
        
            target_card->add_cubes(1);
        }
        
        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                if (origin_card) {
                    return {"STATUS_ADD_CUBE_FOR", origin_card, ncubes};
                } else {
                    return {"STATUS_ADD_CUBE", ncubes};
                }
            } else if (origin_card) {
                return {"STATUS_ADD_CUBE_FOR_OTHER", target, origin_card, ncubes};
            } else {
                return {"STATUS_ADD_CUBE_OTHER", target, ncubes};
            }
        }
    };

    game_string effect_add_cube::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (rn::all_of(origin->cube_slots(), [](card_ptr target) {
            return target->num_cubes == max_cubes;
        })) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_add_cube::on_play(card_ptr origin_card, player_ptr origin) {
        if (int num = std::min<int>(ncubes, origin->m_game->num_cubes)) {
            origin->m_game->queue_request<request_add_cube>(origin_card, origin, num);
        }
    }

    game_string effect_add_cube::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) {
        if (target->num_cubes == max_cubes) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_add_cube::on_play(card_ptr origin_card, player_ptr origin, card_ptr target) {
        target->add_cubes(ncubes);
    }

}