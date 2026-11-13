#include "add_cube.h"

#include "cards/filter_enums.h"

#include "effects/armedanddangerous/ruleset.h"
#include "effects/base/pick.h"

#include "game/game_table.h"

namespace banggame {

    struct request_add_cube : request_picking {
        request_add_cube(card_ptr origin_card, player_ptr target, int ncubes = 1)
            : request_picking(origin_card, nullptr, target, {}, 120)
            , ncubes(ncubes) {}

        int ncubes = 1;

        void on_update() override {
            int nslots = 0;
            int ncards = 0;
            for (card_ptr c : cube_slots(target)) {
                ncards += c->num_cubes() < max_cubes_per_card;
                nslots += max_cubes_per_card - c->num_cubes();
            }

            if (nslots <= ncubes || ncards <= 1) {
                pop_request();
                for (card_ptr c : cube_slots(target)) {
                    int cubes_to_add = std::min<int>(ncubes, max_cubes_per_card - c->num_cubes());
                    ncubes -= cubes_to_add;
                    c->add_cubes(cubes_to_add);
                }
            }
        }
        
        int evaluate_pick(card_ptr target_card) const override {
            if (target_card->has_tag(tag_type::penalty)) {
                return -10;
            } else if (target_card->pocket == pocket_type::player_table) {
                return 5;
            }
            return 0;
        }
        
        bool can_pick(card_ptr target_card) const override {
            return target_card->owner == target
                && (target_card->pocket == pocket_type::player_table && target_card->is_orange()
                || target_card->pocket == pocket_type::player_character && target_card == target->get_character())
                && target_card->num_cubes() < max_cubes_per_card;
        }

        void on_pick(card_ptr target_card) override {
            if (--ncubes == 0) {
                pop_request();
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

    game_string effect_add_cube::on_prompt(card_ptr origin_card, player_ptr origin) const {
        if (rn::all_of(cube_slots(origin), [](card_ptr target) {
            return target->num_cubes() == max_cubes_per_card;
        })) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_add_cube::on_play(card_ptr origin_card, player_ptr origin) const {
        if (int num = std::min<int>(ncubes, origin->m_game->num_tokens(card_token_type::cube))) {
            origin->m_game->queue_request<request_add_cube>(origin_card, origin, num);
        }
    }

    game_string effect_add_cube::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) const {
        if (target->num_cubes() == max_cubes_per_card) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    int effect_add_cube::evaluate(card_ptr origin_card, player_ptr origin, card_ptr target) const {
        if (target->has_tag(tag_type::penalty)) {
            return -5;
        }
        return 0;
    }

    void effect_add_cube::on_play(card_ptr origin_card, player_ptr origin, card_ptr target) const {
        target->add_cubes(ncubes);
    }

    bool effect_min_cubes::can_play(card_ptr origin_card, player_ptr origin) const {
        return origin_card->num_cubes() >= ncubes;
    }

}
