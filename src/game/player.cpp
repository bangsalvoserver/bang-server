#include "player.h"

#include "game.h"

#include "holders.h"
#include "play_verify.h"
#include "game_update.h"

#include "effects/base/damage.h"
#include "effects/base/draw.h"
#include "effects/base/predraw_check.h"
#include "effects/base/requests.h"

#include <cassert>
#include <numeric>

namespace banggame {
    using namespace enums::flag_operators;

    void player::equip_card(card *target) {
        m_game->move_card(target, pocket_type::player_table, this, show_card_flags::shown);
        enable_equip(target);
    }

    void player::enable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            target_card->on_enable(this);
        }
    }

    void player::disable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            target_card->on_disable(this);
        }
    }

    int player::get_initial_cards() {
        return m_game->call_event<event_type::apply_initial_cards_modifier>(this, m_max_hp);
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

    card *player::find_equipped_card(card *card) {
        auto it = std::ranges::find(m_table, card->name, &card::name);
        if (it != m_table.end()) {
            return *it;
        } else {
            return nullptr;
        }
    }

    card *player::random_hand_card() {
        return m_hand[std::uniform_int_distribution(0, static_cast<int>(m_hand.size() - 1))(m_game->rng)];
    }

    static void move_owned_card(player *owner, card *target_card, pocket_type pocket, player *target = nullptr, show_card_flags flags = {}) {
        if (target_card->owner == owner) {
            if (target_card->pocket == pocket_type::player_table) {
                if (target_card->inactive) {
                    target_card->inactive = false;
                    owner->m_game->add_update<game_update_type::tap_card>(target_card, false);
                }
                owner->disable_equip(target_card);
                owner->drop_all_cubes(target_card);
                owner->m_game->move_card(target_card, pocket, target, flags);
                target_card->on_unequip(owner);
            } else if (target_card->pocket == pocket_type::player_hand) {
                owner->m_game->move_card(target_card, pocket, target, flags);
            }
        }
    }

    void player::discard_card(card *target) {
        move_owned_card(this, target, target->color == card_color_type::black
            ? pocket_type::shop_discard
            : pocket_type::discard_pile);
    }

    void player::steal_card(card *target) {
        move_owned_card(target->owner, target, pocket_type::player_hand, this);
    }

    void player::damage(card *origin_card, player *origin, int value, effect_flags flags) {
        effect_damage{value}.on_play(origin_card, origin, this, flags);
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
        m_hp = value;
        m_game->add_update<game_update_type::player_hp>(this, value, instant);
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

    bool player::can_escape(player *origin, card *origin_card, effect_flags flags) const {
        return m_game->call_event<event_type::apply_escapable_modifier>(origin_card, origin, this, flags, false);
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
            disable_equip(origin);
            m_game->move_card(origin, pocket_type::discard_pile);
            m_game->call_event<event_type::on_discard_orange_card>(this, origin);
            origin->on_unequip(this);
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
        m_game->move_card(target, pocket_type::player_hand, this);
    }

    void player::add_to_hand_phase_one(card *drawn_card) {
        ++m_num_drawn_cards;
        
        bool reveal = m_game->call_event<event_type::on_card_drawn>(this, drawn_card, false);
        if (drawn_card->pocket == pocket_type::discard_pile) {
            m_game->add_log("LOG_DRAWN_FROM_DISCARD", this, drawn_card);
        } else if (reveal) {
            m_game->add_log("LOG_DRAWN_CARD", this, drawn_card);
            m_game->send_card_update(drawn_card, this, show_card_flags::shown | show_card_flags::short_pause);
        } else {
            m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD", this);
            m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
        }
        m_game->move_card(drawn_card, pocket_type::player_hand, this);
    }

    void player::draw_card(int ncards, card *origin_card) {
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
        for (int i=0; i<ncards; ++i) {
            card *drawn_card = m_game->m_deck.back();
            if (origin_card) {
                m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
            } else {
                m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
            }
            m_game->draw_card_to(pocket_type::player_hand, this);
        }
    }

    void player::set_last_played_card(card *c) {
        m_last_played_card = c;
        m_game->add_update<game_update_type::last_played_card>(update_target::includes_private(this), c);
    }

    bool player::is_bangcard(card *origin_card) {
        return (check_player_flags(player_flags::treat_missed_as_bang)
                && origin_card->has_tag(tag_type::missedcard))
            || origin_card->has_tag(tag_type::bangcard);
    };

    game_string player::handle_action(enums::enum_tag_t<game_action_type::pick_card>, const picking_args &args) {
        if (m_prompt) {
            return "ERROR_MUST_RESPOND_PROMPT";
        } else if (!m_game->pending_requests()) {
            return "ERROR_NO_PENDING_REQUEST";
        } else if (auto &req = m_game->top_request(); req.target() != this) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        } else {
            m_game->add_update<game_update_type::confirm_play>(update_target::includes_private(this));
            if (req.can_pick(args.pocket, args.player, args.card)) {
                req.on_pick(args.pocket, args.player, args.card);
                return {};
            } else {
                return "ERROR_INVALID_PICK";
            }
        }
    }

    void player::play_card_action(card *origin_card) {
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
            m_game->move_card(origin_card, pocket_type::discard_pile);
            m_game->call_event<event_type::on_play_hand_card>(this, origin_card);
            break;
        case pocket_type::player_table:
            if (origin_card->color == card_color_type::green) {
                m_game->move_card(origin_card, pocket_type::discard_pile);
            }
            break;
        case pocket_type::shop_selection:
            if (origin_card->color == card_color_type::brown) {
                m_game->move_card(origin_card, pocket_type::shop_discard);
            }
            break;
        default:
            break;
        }
    }

    void player::log_played_card(card *origin_card, bool is_response) {
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
            m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_CARD", origin_card, this);
            break;
        case pocket_type::player_table:
            m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_TABLE_CARD", origin_card, this);
            break;
        case pocket_type::player_character:
            m_game->add_log(is_response ?
                origin_card->has_tag(tag_type::drawing)
                    ? "LOG_DRAWN_WITH_CHARACTER"
                    : "LOG_RESPONDED_WITH_CHARACTER"
                : "LOG_PLAYED_CHARACTER", origin_card, this);
            break;
        case pocket_type::shop_selection:
            m_game->add_log("LOG_BOUGHT_CARD", origin_card, this);
            break;
        }
    }

    game_string player::handle_action(enums::enum_tag_t<game_action_type::play_card>, const play_card_args &args) {
        if (m_prompt) {
            return "ERROR_MUST_RESPOND_PROMPT";
        }
        return play_card_verify{this, args.card, false, args.targets, unwrap_vector_not_null(args.modifiers)}.verify_and_play();
    }
    
    game_string player::handle_action(enums::enum_tag_t<game_action_type::respond_card>, const play_card_args &args) {
        if (m_prompt) {
            return "ERROR_MUST_RESPOND_PROMPT";
        }
        return play_card_verify{this, args.card, true, args.targets, unwrap_vector_not_null(args.modifiers)}.verify_and_respond();
    }

    void player::prompt_then(game_string &&message, std::function<void()> &&fun) {
        if (message) {
            m_prompt.emplace(std::move(fun), message);
            m_game->add_update<game_update_type::game_prompt>(update_target::includes_private(this), std::move(message));
        } else {
            if (m_game->pending_requests() && bool(m_game->top_request().flags() & effect_flags::force_play)) {
                m_game->pop_request();
            }
            m_game->add_update<game_update_type::confirm_play>(update_target::includes_private(this));
            std::invoke(fun);
        }
    }

    game_string player::handle_action(enums::enum_tag_t<game_action_type::prompt_respond>, bool response) {
        if (!m_prompt) {
            return "ERROR_NO_PROMPT";
        }
        auto fun = std::move(m_prompt->first);
        m_prompt.reset();

        if (response && m_game->pending_requests() && bool(m_game->top_request().flags() & effect_flags::force_play)) {
            m_game->pop_request();
        }
        m_game->add_update<game_update_type::confirm_play>(update_target::includes_private(this));
        if (response) {
            std::invoke(fun);
        } else if (m_game->pending_requests()) {
            m_game->update_request();
        }
        return {};
    }

    game_string player::handle_action(enums::enum_tag_t<game_action_type::request_confirm>) {
        if (m_game->pending_requests()) {
            m_game->top_request().confirm_player(this);
        }
        return {};
    }

    void player::draw_from_deck() {
        m_game->call_event<event_type::on_draw_from_deck>(this);
        if (m_game->top_request_is<request_draw>()) {
            m_game->pop_request();
            int ncards = get_cards_to_draw();
            while (m_num_drawn_cards < ncards) {
                add_to_hand_phase_one(m_game->phase_one_drawn_card());
            }
            m_game->update_request();
        }
        m_game->queue_action([this]{
            m_game->call_event<event_type::post_draw_cards>(this);
        });
    }

    card_sign player::get_card_sign(card *target_card) {
        return m_game->call_event<event_type::apply_sign_modifier>(this, target_card->sign);
    }

    void player::start_of_turn() {
        m_game->m_playing = this;

        m_num_drawn_cards = 0;
        
        for (auto &[card_id, obj] : m_predraw_checks) {
            obj.resolved = false;
        }
        
        m_game->add_update<game_update_type::switch_turn>(this);
        m_game->add_log("LOG_TURN_START", this);
        m_game->call_event<event_type::pre_turn_start>(this);

        m_game->queue_action([this]{ next_predraw_check(); });
    }

    void player::next_predraw_check() {
        if (alive() && m_game->m_playing == this && !m_game->check_flags(game_flags::game_over)) {
            if (std::ranges::all_of(m_predraw_checks | std::views::values, &predraw_check::resolved)) {
                request_drawing();
            } else {
                effect_predraw_check::queue(this);
            }
        }
    }

    void player::request_drawing() {
        m_game->call_event<event_type::on_turn_start>(this);
        m_game->queue_action([this]{
            if (m_game->check_flags(game_flags::phase_one_override)) {
                m_game->call_event<event_type::phase_one_override>(this);
            } else {
                m_game->queue_request<request_draw>(this);
            }
        });
    }

    void player::pass_turn() {
        if (m_hand.size() > max_cards_end_of_turn()) {
            m_game->queue_request<request_discard_pass>(this);
        } else {
            untap_inactive_cards();

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
        untap_inactive_cards();
        remove_player_flags(player_flags::extra_turn);
        m_game->call_event<event_type::on_turn_end>(this, true);
        m_game->start_next_turn();
    }

    void player::untap_inactive_cards() {
        for (card *c : m_table) {
            if (c->inactive) {
                c->inactive = false;
                m_game->add_update<game_update_type::tap_card>(c, false);
            }
        }
    }

    void player::remove_extra_characters() {
        if (m_characters.size() <= 1) return;

        m_game->add_update<game_update_type::remove_cards>(to_vector_not_null(m_characters | std::views::drop(1)));
        m_characters.resize(1);
    }

    void player::discard_all(bool death) {
        if (!only_black_cards_equipped()) {
            untap_inactive_cards();
            if (death) {
                m_game->queue_request_front<request_discard_all>(this);
            } else {
                m_game->queue_request_front<request_sheriff_killed_deputy>(this);
            }
        }
        m_game->queue_action_front([this]{
            std::vector<card *> black_cards;
            for (card *c : m_table) {
                if (c->color == card_color_type::black) {
                    black_cards.push_back(c);
                }
            }
            for (card *c : black_cards) {
                m_game->add_log("LOG_DISCARDED_SELF_CARD", this, c);
                discard_card(c);
            }
            add_gold(-m_gold);
            drop_all_cubes(m_characters.front());
        });
    }

    bool player::only_black_cards_equipped() const {
        return std::ranges::all_of(m_table, [](card *c) {
            return c->color == card_color_type::black;
        }) && m_hand.empty();
    }

    void player::set_role(player_role role) {
        m_role = role;

        if (role == player_role::sheriff || m_game->m_players.size() <= 3) {
            m_game->add_update<game_update_type::player_show_role>(this, m_role, true);
            add_player_flags(player_flags::role_revealed);
        } else {
            m_game->add_update<game_update_type::player_show_role>(update_target::includes(this), this, m_role, true);
        }
    }

    void player::reset_max_hp() {
        m_max_hp = m_characters.front()->get_tag_value(tag_type::max_hp).value_or(4) + (m_role == player_role::sheriff);
    }

    void player::send_player_status() {
        m_game->add_update<game_update_type::player_status>(this, m_player_flags, m_range_mod, m_weapon_range, m_distance_mod);
    }

    bool player::add_player_flags(player_flags flags) {
        if (!check_player_flags(flags)) {
            m_player_flags |= flags;
            send_player_status();
            return true;
        }
        return false;
    }

    bool player::remove_player_flags(player_flags flags) {
        if (check_player_flags(flags)) {
            m_player_flags &= ~flags;
            send_player_status();
            return true;
        }
        return false;
    }

    bool player::check_player_flags(player_flags flags) const {
        return (m_player_flags & flags) == flags;
    }

    int player::count_cubes() const {
        return m_characters.front()->num_cubes
            + std::transform_reduce(
                m_table.begin(),
                m_table.end(),
                0, std::plus(),
                std::mem_fn(&card::num_cubes));
    }
}