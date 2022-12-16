#include "add_cube.h"

#include "game/game.h"

namespace banggame {
    
    struct request_add_cube : request_base {
        request_add_cube(card *origin_card, player *target, int ncubes = 1)
            : request_base(origin_card, nullptr, target)
            , ncubes(std::min<int>(ncubes, target->m_game->num_cubes)) {}

        int ncubes = 1;

        void on_update() override {
            int nslots = 0;
            int ncards = 0;
            for (card *c : target->cube_slots()) {
                ncards += c->num_cubes < max_cubes;
                nslots += max_cubes - c->num_cubes;
            }

            if (nslots <= ncubes || ncards <= 1) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    for (card *c : target->cube_slots()) {
                        int cubes_to_add = std::min<int>(ncubes, max_cubes - c->num_cubes);
                        ncubes -= cubes_to_add;
                        target->add_cubes(c, cubes_to_add);
                    }
                });
            }
        }
        
        bool can_pick(card *target_card) const override {
            if (target_card->owner == target) {
                if (target_card->pocket == pocket_type::player_table && target_card->is_orange()) {
                    return target_card->num_cubes < max_cubes;
                } else if (target_card->pocket == pocket_type::player_character) {
                    return target->first_character()->num_cubes < max_cubes;
                }
            }
            return false;
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                if (--ncubes == 0) {
                    target->m_game->pop_request();
                }
            
                target->add_cubes(target_card->pocket == pocket_type::player_character
                    ? target->first_character()
                    : target_card, 1);
            });
        }
        
        game_string status_text(player *owner) const override {
            if (owner == target) {
                if (ncubes == 1) {
                    if (origin_card) {
                        return {"STATUS_ADD_CUBE_FOR", origin_card};
                    } else {
                        return "STATUS_ADD_CUBE";
                    }
                } else if (origin_card) {
                    return {"STATUS_ADD_CUBE_PLURAL_FOR", origin_card, ncubes};
                } else {
                    return {"STATUS_ADD_CUBE_PLURAL", ncubes};
                }
            } else if (ncubes == 1) {
                if (origin_card) {
                    return {"STATUS_ADD_CUBE_FOR_OTHER", target, origin_card};
                } else {
                    return {"STATUS_ADD_CUBE_OTHER", target};
                }
            } else if (origin_card) {
                return {"STATUS_ADD_CUBE_PLURAL_FOR_OTHER", target, origin_card, ncubes};
            } else {
                return {"STATUS_ADD_CUBE_PLURAL_OTHER", target, ncubes};
            }
        }
    };

    game_string effect_add_cube::on_prompt(card *origin_card, player *origin) {
        if (std::ranges::all_of(origin->cube_slots(), [](card *target) {
            return target->num_cubes == max_cubes;
        })) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_add_cube::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_add_cube>(origin_card, origin, ncubes);
    }

    game_string effect_add_cube::on_prompt(card *origin_card, player *origin, card *target) {
        if (target->num_cubes == max_cubes) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_add_cube::on_play(card *origin_card, player *origin, card *target) {
        target->owner->add_cubes(target, ncubes);
    }

}