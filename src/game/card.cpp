#include "card.h"
#include "game.h"

#include "effects/armedanddangerous/ruleset.h"

namespace banggame {

    card_backface::card_backface(card *c): id(c->id), deck(c->deck) {}

    void card::set_visibility(card_visibility new_visibility, player *new_owner, bool instant) {
        if (new_visibility == card_visibility::hidden) {
            if (visibility == card_visibility::show_owner) {
                m_game->add_update<game_update_type::hide_card>(update_target::includes(owner), this, instant);
            } else if (visibility == card_visibility::shown) {
                m_game->add_update<game_update_type::hide_card>(this, instant);
            }
            visibility = card_visibility::hidden;
        } else if (!new_owner || new_visibility == card_visibility::shown) {
            if (visibility == card_visibility::show_owner) {
                m_game->add_update<game_update_type::show_card>(update_target::excludes(owner), this, *this, instant);
            } else if (visibility == card_visibility::hidden) {
                m_game->add_update<game_update_type::show_card>(this, *this, instant);
            }
            visibility = card_visibility::shown;
        } else if (owner != new_owner || visibility != card_visibility::show_owner) {
            if (visibility == card_visibility::shown) {
                m_game->add_update<game_update_type::hide_card>(update_target::excludes(new_owner), this, instant);
            } else {
                if (visibility == card_visibility::show_owner) {
                    m_game->add_update<game_update_type::hide_card>(update_target::includes(owner), this, instant);
                }
                m_game->add_update<game_update_type::show_card>(update_target::includes(new_owner), this, *this, instant);
            }
            visibility = card_visibility::show_owner;
        }
    }

    void card::move_to(pocket_type new_pocket, player *new_owner, card_visibility new_visibility, bool instant, bool front) {
        if (pocket == new_pocket && owner == new_owner) return;
        
        set_visibility(new_visibility, new_owner, instant);

        auto &prev_pile = m_game->get_pocket(pocket, owner);
        prev_pile.erase(rn::find(prev_pile, this));

        pocket = new_pocket;
        owner = new_owner;

        auto &new_pile = m_game->get_pocket(new_pocket, new_owner);
        if (front) {
            new_pile.insert(new_pile.begin(), this);
        } else {
            new_pile.push_back(this);
        }
        
        m_game->add_update<game_update_type::move_card>(this, new_owner, new_pocket, instant, front);
    }

    void card::set_inactive(bool new_inactive) {
        if (new_inactive != inactive) {
            m_game->add_update<game_update_type::tap_card>(this, new_inactive);
            inactive = new_inactive;
        }
    }

    void card::flash_card() {
        m_game->add_update<game_update_type::flash_card>(this);
    }

    void card::add_short_pause() {
        m_game->add_update<game_update_type::short_pause>(this);
    }
    
    void card::add_cubes(int ncubes) {
        ncubes = std::min<int>({ncubes, num_cubes, max_cubes - num_cubes});
        if (ncubes > 0) {
            num_cubes -= ncubes;
            num_cubes += ncubes;
            m_game->add_log("LOG_ADD_CUBE", owner, this, ncubes);
            m_game->add_update<game_update_type::move_cubes>(ncubes, nullptr, this);
        }
    }

    void card::move_cubes(card *target, int ncubes, bool instant) {
        ncubes = std::min<int>(ncubes, num_cubes);
        if (target && ncubes > 0 && target->num_cubes < max_cubes) {
            int added_cubes = std::min<int>(ncubes, max_cubes - target->num_cubes);
            target->num_cubes += added_cubes;
            num_cubes -= added_cubes;
            ncubes -= added_cubes;
            if (owner == target->owner) {
                m_game->add_log("LOG_MOVED_CUBE", target->owner, this, target, added_cubes);
            } else {
                m_game->add_log("LOG_MOVED_CUBE_FROM", target->owner, owner, this, target, added_cubes);
            }
            m_game->add_update<game_update_type::move_cubes>(added_cubes, this, target, instant);
        }
        if (ncubes > 0) {
            num_cubes -= ncubes;
            m_game->num_cubes += ncubes;
            m_game->add_log("LOG_PAID_CUBE", owner, this, ncubes);
            m_game->add_update<game_update_type::move_cubes>(ncubes, this, nullptr, instant);
        }
        if (sign && num_cubes == 0) {
            m_game->add_log("LOG_DISCARDED_ORANGE_CARD", owner, this);
            m_game->call_event(event_type::on_discard_orange_card{ owner, this });
            owner->disable_equip(this);
            move_to(pocket_type::discard_pile);
        }
    }

    void card::drop_cubes() {
        if (num_cubes > 0) {
            m_game->add_log("LOG_DROP_CUBE", owner, this, num_cubes);
            m_game->num_cubes += num_cubes;
            m_game->add_update<game_update_type::move_cubes>(num_cubes, this, nullptr);
            num_cubes = 0;
        }
    }
}