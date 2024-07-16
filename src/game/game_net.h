#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <numeric>
#include <iostream>

#include "player.h"
#include "game_update.h"

#include "utils/id_map.h"

namespace banggame {

    class update_target {
    private:
        const player *m_targets[lobby_max_players];

        struct {
            bool m_inclusive:1;
            bool m_invert_public:1;
            int m_num_targets:6;
        };

        update_target(bool inclusive, bool invert_public, std::convertible_to<const player *> auto ... targets)
            : m_targets{targets ...}
            , m_inclusive{inclusive}
            , m_invert_public{invert_public}
            , m_num_targets{sizeof...(targets)}
        {
            static_assert(sizeof...(targets) <= lobby_max_players);
        }

        update_target(bool inclusive, std::convertible_to<const player *> auto ... targets)
            : update_target(inclusive, false, targets...) {}

        auto targets() const {
            return std::span{m_targets, m_targets + m_num_targets};
        }

    public:
        static update_target includes(std::convertible_to<const player *> auto ... targets) {
            return update_target(true, targets...);
        }

        static update_target excludes(std::convertible_to<const player *> auto ... targets) {
            return update_target(false, targets...);
        }

        static update_target includes_private(std::convertible_to<const player *> auto ... targets) {
            return update_target(true, true, targets...);
        }

        static update_target excludes_public(std::convertible_to<const player *> auto ... targets) {
            return update_target(false, true, targets...);
        }

        void add(player *target) {
            m_targets[m_num_targets++] = target;
        }

        bool matches(const player *target) const {
            return rn::contains(targets(), target) == m_inclusive;
        }

        bool matches(int user_id) const {
            return rn::contains(targets(), user_id, &player::user_id) == m_inclusive;
        }

        bool is_public() const {
            return m_invert_public != m_inclusive != (m_num_targets == 0);
        }
    };

    struct game_context {
        virtual card *find_card(int card_id) const = 0;
        virtual player *find_player(int player_id) const = 0;
        virtual player *find_player_by_userid(int user_id) const = 0;
        virtual game_duration transform_duration(game_duration duration) const = 0;
    };

    struct game_update_tuple {
        update_target target;
        json::json content;
        game_duration duration;
    };

    class game_net_manager : public game_context {
    protected:
        std::deque<game_update_tuple> m_updates;
        std::deque<std::pair<update_target, game_string>> m_saved_log;

    private:
        json::json serialize_update(const game_update &update) const;

    protected:
        template<utils::tstring E> requires game_update_type<E>
        json::json make_update(auto && ... args) {
            return serialize_update(game_update{utils::tag<E>{}, FWD(args) ... });
        }
    
    public:
        bool pending_updates() const {
            return !m_updates.empty();
        }

        game_update_tuple get_next_update() {
            auto update = std::move(m_updates.front());
            m_updates.pop_front();
            return update;
        }

        std::string handle_game_action(int user_id, const json::json &value);

    public:
        template<utils::tstring E> requires game_update_type<E>
        void add_update(update_target target, auto && ... args) {
            using value_type = utils::tagged_variant_value_type<game_update, utils::tag<E>>;
            game_duration duration{};
            if constexpr (std::is_void_v<value_type>) {
                m_updates.emplace_back(target, serialize_update(game_update{utils::tag<E>{}}), duration);
            } else {
                value_type update{FWD(args) ...};
                if constexpr (requires { value_type::duration; }) {
                    duration = update.duration.get();
                }
                m_updates.emplace_back(target, serialize_update(game_update{utils::tag<E>{}, std::move(update)}), duration);
            }
        }

        template<utils::tstring E> requires game_update_type<E>
        void add_update(auto && ... args) {
            add_update<E>(update_target::excludes(), FWD(args) ... );
        }

        template<size_t N, typename ... Ts>
        void add_log(update_target target, const char (&message)[N], Ts && ... args) {
            const auto &log = m_saved_log.emplace_back(target, game_string(message, FWD(args) ...));
            add_update<"game_log">(std::move(target), log.second);
        }

        template<size_t N, typename ... Ts>
        void add_log(const char (&message)[N], Ts && ... args) {
            add_log(update_target::excludes(), message, FWD(args) ... );
        }
    };

}

#endif