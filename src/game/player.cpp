#include "player.h"

#include "game.h"

#include "play_verify.h"
#include "game_update.h"

#include "cards/holders.h"
#include "cards/filters.h"
#include "cards/game_enums.h"
#include "cards/effect_context.h"

#include "cards/base/damage.h"
#include "cards/base/draw.h"
#include "cards/base/predraw_check.h"
#include "cards/base/requests.h"

#include <cassert>
#include <numeric>

namespace banggame {

    inline card_pocket_pair to_card_pocket_pair(card *c) {
        return {c, c->pocket};
    }

    void effect_context_deleter::operator ()(effect_context *ctx) const noexcept {
        delete ctx;
    }

    shared_effect_context make_shared_effect_context(effect_context &&ctx) {
        return shared_effect_context(new effect_context(std::move(ctx)), effect_context_deleter{});
    }

    played_card_history::played_card_history(card *origin_card, const modifier_list &modifiers, const effect_context &context)
        : origin_card{to_card_pocket_pair(origin_card)}
        , modifiers{modifiers
            | ranges::views::transform(&modifier_pair::card)
            | ranges::views::transform(to_card_pocket_pair)
            | ranges::to<std::vector>}
        , context{new effect_context(context)} {}

    bool player::is_bot() const {
        return user_id < 0;
    }

    bool player::is_ghost() const {
        return check_player_flags(player_flags::dead)
            && filters::is_player_ghost(this);
    }

    bool player::alive() const {
        return filters::is_player_alive(this);
    }

    void player::equip_card(card *target) {
        m_game->move_card(target, pocket_type::player_table, this, card_visibility::shown);
        enable_equip(target);
    }

    void player::enable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            for (const equip_holder &e : target_card->equips) {
                e.on_enable(target_card, this);
            }
        }
    }

    void player::disable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            for (const equip_holder &e : target_card->equips) {
                e.on_disable(target_card, this);
            }
        }
    }

    int player::get_initial_cards() {
        return first_character()->get_tag_value(tag_type::initial_cards).value_or(m_max_hp);
    }

    int player::max_cards_end_of_turn() {
        return m_game->call_event<event_type::apply_maxcards_modifier>(this, m_hp);
    }

    int player::get_num_checks() {
        return m_game->call_event<event_type::count_num_checks>(this, 1);
    }

    int player::get_bangs_played() {
        return m_game->call_event<event_type::count_bangs_played>(this, 0);
    }

    int player::get_cards_to_draw() {
        return m_game->call_event<event_type::count_cards_to_draw>(this, 2);
    }

    int player::get_range_mod() const {
        return m_game->call_event<event_type::count_range_mod>(this, range_mod_type::range_mod, 0);
    }

    int player::get_weapon_range() const {
        return m_game->call_event<event_type::count_range_mod>(this, range_mod_type::weapon_range, 1);
    }

    int player::get_distance_mod() const {
        return m_game->call_event<event_type::count_range_mod>(this, range_mod_type::distance_mod, 0);
    }

    card *player::find_equipped_card(card *card) {
        auto it = std::ranges::find(m_table, card->name, &card::name);
        if (it != m_table.end()) {
            return *it;
        } else {
            return nullptr;
        }
    }

    card *player::random_hand_card() {
        return m_hand[std::uniform_int_distribution(0, int(m_hand.size() - 1))(m_game->rng)];
    }

    static bool move_owned_card(player *owner, card *target_card, bool used) {
        if (target_card->owner == owner) {
            if (target_card->pocket == pocket_type::player_table) {
                owner->m_game->tap_card(target_card, false);
                owner->disable_equip(target_card);
                owner->drop_all_cubes(target_card);
                return true;
            } else if (target_card->pocket == pocket_type::player_hand) {
                owner->m_game->call_event<event_type::on_discard_hand_card>(owner, target_card, used);
                return true;
            }
        }
    }

    void player::discard_card(card *target, bool used) {
        if (move_owned_card(this, target, used)) {
            if (target->is_train()) {
                if (m_game->m_train.size() < 4) {
                    m_game->move_card(target, pocket_type::train);
                } else {
                    m_game->move_card(target, pocket_type::train_deck, nullptr, card_visibility::hidden);
                }
            } else if (target->is_black()) {
                m_game->move_card(target, pocket_type::shop_discard);
            } else {
                m_game->move_card(target, pocket_type::discard_pile);
            }
        }
    }

    void player::steal_card(card *target) {
        if (target->owner != this || target->pocket != pocket_type::player_table || !target->is_train()) {
            if (move_owned_card(target->owner, target, false)) {
                add_to_hand(target);
            }
        }
    }

    void player::damage(card *origin_card, player *origin, int value, effect_flags flags) {
        m_game->queue_request_front<request_damage>(origin_card, origin, this, value, flags);
    }

    void player::heal(int value) {
        if (is_ghost() || m_hp == m_max_hp) return;
        
        if (value == 1) {
            m_game->add_log("LOG_HEALED", this);
        } else {
            m_game->add_log("LOG_HEALED_PLURAL", this, value);
        }
        set_hp(std::min<int>(m_hp + value, m_max_hp));
    }

    void player::set_hp(int value, bool instant) {
        if (value != m_hp) {
            m_hp = value;
            m_game->add_update<game_update_type::player_hp>(this, value, instant);
        }
    }

    void player::add_gold(int amount) {
        if (amount) {
            m_gold += amount;
            m_game->add_update<game_update_type::player_gold>(this, m_gold);
        }
    }

    bool player::immune_to(card *origin_card, player *origin, effect_flags flags) const {
        return m_game->call_event<event_type::apply_immunity_modifier>(origin_card, origin, this, flags, false);
    }

    int player::can_escape(player *origin, card *origin_card, effect_flags flags) const {
        return m_game->call_event<event_type::apply_escapable_modifier>(origin_card, origin, this, flags, 0);
    }
    
    void player::add_cubes(card *target, int ncubes) {
        ncubes = std::min<int>({ncubes, m_game->num_cubes, max_cubes - target->num_cubes});
        if (ncubes > 0) {
            m_game->num_cubes -= ncubes;
            target->num_cubes += ncubes;
            m_game->add_update<game_update_type::move_cubes>(ncubes, nullptr, target);
        }
    }

    void player::pay_cubes(card *origin, int ncubes) {
        move_cubes(origin, nullptr, ncubes);
    }

    void player::move_cubes(card *origin, card *target, int ncubes) {
        ncubes = std::min<int>(ncubes, origin->num_cubes);
        if (target && ncubes > 0 && target->num_cubes < max_cubes) {
            int added_cubes = std::min<int>(ncubes, max_cubes - target->num_cubes);
            target->num_cubes += added_cubes;
            origin->num_cubes -= added_cubes;
            ncubes -= added_cubes;
            m_game->add_update<game_update_type::move_cubes>(added_cubes, origin, target);
        }
        if (ncubes > 0) {
            origin->num_cubes -= ncubes;
            m_game->num_cubes += ncubes;
            m_game->add_update<game_update_type::move_cubes>(ncubes, origin, nullptr);
        }
        if (origin->sign && origin->num_cubes == 0) {
            m_game->add_log("LOG_DISCARDED_ORANGE_CARD", this, origin);
            m_game->call_event<event_type::on_discard_orange_card>(this, origin);
            disable_equip(origin);
            m_game->move_card(origin, pocket_type::discard_pile);
        }
    }

    void player::drop_all_cubes(card *target) {
        if (target->num_cubes > 0) {
            m_game->num_cubes += target->num_cubes;
            m_game->add_update<game_update_type::move_cubes>(target->num_cubes, target, nullptr);
            target->num_cubes = 0;
        }
    }

    void player::add_to_hand(card *target) {
        if (target->deck == card_deck_type::train) {
            equip_card(target);
        } else {
            m_game->move_card(target, pocket_type::player_hand, this, m_game->check_flags(game_flags::hands_shown)
                ? card_visibility::shown : card_visibility::show_owner);
        }
    }

    void player::add_to_hand_phase_one(card *drawn_card) {
        ++m_num_drawn_cards;
        
        bool reveal = m_game->call_event<event_type::on_card_drawn>(this, drawn_card, false);
        if (drawn_card->pocket == pocket_type::discard_pile) {
            m_game->add_log("LOG_DRAWN_FROM_DISCARD", this, drawn_card);
        } else if (m_game->check_flags(game_flags::hands_shown)) {
            m_game->add_log("LOG_DRAWN_CARD", this, drawn_card);
        } else if (reveal) {
            m_game->add_log("LOG_DRAWN_CARD", this, drawn_card);
            m_game->set_card_visibility(drawn_card);
            m_game->add_short_pause(drawn_card);
        } else {
            m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD", this);
            m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
        }
        add_to_hand(drawn_card);
    }

    void player::draw_card(int ncards, card *origin_card) {
        if (!m_game->check_flags(game_flags::hands_shown)) {
            if (ncards == 1) {
                if (origin_card) {
                    m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD_FOR", this, origin_card);
                } else {
                    m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD", this);
                }
            } else {
                if (origin_card) {
                    m_game->add_log(update_target::excludes(this), "LOG_DRAWN_N_CARDS_FOR", this, ncards, origin_card);
                } else {
                    m_game->add_log(update_target::excludes(this), "LOG_DRAWN_N_CARDS", this, ncards);
                }
            }
        }
        for (int i=0; i<ncards; ++i) {
            card *drawn_card = m_game->top_of_deck();
            if (m_game->check_flags(game_flags::hands_shown)) {
                if (origin_card) {
                    m_game->add_log("LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
                } else {
                    m_game->add_log("LOG_DRAWN_CARD", this, drawn_card);
                }
            } else {
                if (origin_card) {
                    m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
                } else {
                    m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
                }
            }
            add_to_hand(drawn_card);
        }
    }

    void player::handle_game_action(const game_action &action) {
        enums::visit_indexed(overloaded{
            [](enums::enum_tag_t<message_type::ok>) {},
            [&](enums::enum_tag_t<message_type::error>, game_string message) {
                m_game->add_update<game_update_type::game_error>(update_target::includes_private(this), std::move(message));
            },
            [&](enums::enum_tag_t<message_type::prompt>, game_string message) {
                m_game->add_update<game_update_type::game_prompt>(update_target::includes_private(this), std::move(message));
            }
        }, enums::visit(overloaded{
            [&](const pick_card_args &args) { return verify_and_pick(this, args); },
            [&](const play_card_args &args) { return verify_and_play(this, args); }
        }, action));
    }

    void player::start_of_turn() {
        m_game->m_playing = this;
        m_game->queue_action([this]{
            m_game->add_update<game_update_type::switch_turn>(this);
            m_game->add_log("LOG_TURN_START", this);
            m_game->call_event<event_type::pre_turn_start>(this);
        }, -7);
        m_game->queue_action([this]{
            m_played_cards.clear();
            m_num_drawn_cards = 0;
            for (auto &[card_id, obj] : m_predraw_checks) {
                obj.resolved = false;
            }
            request_drawing();
        }, -7);
    }

    void player::request_drawing() {
        if (alive() && m_game->m_playing == this) {
            if (std::ranges::all_of(m_predraw_checks | std::views::values, &predraw_check::resolved)) {
                m_game->call_event<event_type::on_turn_start>(this);
                m_game->queue_action([this]{
                    if (m_game->check_flags(game_flags::phase_one_override)) {
                        m_game->call_event<event_type::on_draw_from_deck>(this);
                    } else {
                        m_game->queue_request<request_draw>(this);
                    }
                });
            } else {
                m_game->queue_request<request_predraw>(this);
            }
        }
    }

    void player::pass_turn() {
        if (m_hand.size() > max_cards_end_of_turn()) {
            m_game->queue_request<request_discard_pass>(this);
        } else {
            m_game->call_event<event_type::on_turn_end>(this, false);
            m_game->queue_action([&]{
                if (m_extra_turns == 0) {
                    remove_player_flags(player_flags::extra_turn);
                    m_game->start_next_turn();
                } else {
                    --m_extra_turns;
                    add_player_flags(player_flags::extra_turn);
                    start_of_turn();
                }
            });
        }
    }

    void player::skip_turn() {
        remove_player_flags(player_flags::extra_turn);
        m_game->call_event<event_type::on_turn_end>(this, true);
        m_game->start_next_turn();
    }

    void player::remove_extra_characters() {
        if (auto range = m_characters | std::views::drop(1)) {
            m_game->add_update<game_update_type::remove_cards>(ranges::to<serial::card_list>(range));

            for (card *character : range) {
                disable_equip(character);
                
                character->pocket = pocket_type::none;
                character->owner = nullptr;
                character->visibility = card_visibility::hidden;
            }

            m_characters.resize(1);
        }
    }

    void player::set_role(player_role role, bool instant) {
        m_role = role;

        if (role == player_role::sheriff || m_game->m_players.size() <= 3 || check_player_flags(player_flags::role_revealed)) {
            m_game->add_update<game_update_type::player_show_role>(this, m_role, instant);
            add_player_flags(player_flags::role_revealed);
        } else {
            m_game->add_update<game_update_type::player_show_role>(update_target::includes(this), this, m_role, instant);
        }
    }

    void player::reset_max_hp() {
        m_max_hp = first_character()->get_tag_value(tag_type::max_hp).value_or(4) + (m_role == player_role::sheriff);
    }

    bool player::add_player_flags(player_flags flags) {
        if (!check_player_flags(flags)) {
            m_player_flags |= flags;
            m_game->add_update<game_update_type::player_flags>(this, m_player_flags);
            return true;
        }
        return false;
    }

    bool player::remove_player_flags(player_flags flags) {
        if (check_player_flags(flags)) {
            m_player_flags &= ~flags;
            m_game->add_update<game_update_type::player_flags>(this, m_player_flags);
            return true;
        }
        return false;
    }

    bool player::check_player_flags(player_flags flags) const {
        return (m_player_flags & flags) == flags;
    }

    int player::count_cubes() const {
        return ranges::accumulate(cube_slots()
            | ranges::views::transform(&card::num_cubes), 0);
    }
}