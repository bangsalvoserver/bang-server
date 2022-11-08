#include "add_cube.h"

#include "game/game.h"

namespace banggame {
    
    struct request_add_cube : request_base {
        request_add_cube(card *origin_card, player *target, int ncubes = 1)
            : request_base(origin_card, nullptr, target)
            , ncubes(ncubes) {}

        int ncubes = 1;
        
        bool can_pick(card *target_card) const override {
            if (target_card->owner == target) {
                if (target_card->pocket == pocket_type::player_table && target_card->color == card_color_type::orange) {
                    return target_card->num_cubes < max_cubes;
                } else if (target_card->pocket == pocket_type::player_character) {
                    return target->m_characters.front()->num_cubes < max_cubes;
                }
            }
            return false;
        }

        void on_pick(card *target_card) override {
            if (target_card->pocket == pocket_type::player_character) {
                target_card = target->m_characters.front();
            }
            if (--ncubes == 0) {
                target->m_game->pop_request();
            }
            target->add_cubes(target_card, 1);
            target->m_game->update_request();
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

    void effect_add_cube::on_play(card *origin_card, player *origin) {
        int nslots = max_cubes - origin->m_characters.front()->num_cubes;
        int ncards = nslots > 0;
        for (card *c : origin->m_table) {
            if (c->color == card_color_type::orange) {
                ncards += c->num_cubes < max_cubes;
                nslots += max_cubes - c->num_cubes;
            }
        }
        ncubes = std::min<int>(ncubes, origin->m_game->num_cubes);
        if (nslots <= ncubes || ncards <= 1) {
            auto do_add_cubes = [&](card *c) {
                int cubes_to_add = std::min<int>(ncubes, max_cubes - c->num_cubes);
                ncubes -= cubes_to_add;
                origin->add_cubes(c, cubes_to_add);
            };
            do_add_cubes(origin->m_characters.front());
            for (card *c : origin->m_table) {
                if (c->color == card_color_type::orange) {
                    do_add_cubes(c);
                }
            }
        } else if (ncubes > 0) {
            origin->m_game->queue_request<request_add_cube>(origin_card, origin, ncubes);
        }
    }

    void effect_add_cube::on_play(card *origin_card, player *origin, card *target) {
        target->owner->add_cubes(target, ncubes);
    }

}