#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <numeric>
#include <iostream>

#include "player.h"
#include "game_string.h"
#include "game_update.h"

#include "utils/id_map.h"

namespace banggame {

    class update_target {
    private:
        player *m_targets[lobby_max_players];

        struct {
            bool m_inclusive:1;
            bool m_invert_public:1;
            int m_num_targets:6;
        };

        update_target(bool inclusive, bool invert_public, std::same_as<player *> auto ... targets)
            : m_targets{targets ...}
            , m_inclusive{inclusive}
            , m_invert_public{invert_public}
            , m_num_targets{sizeof...(targets)}
        {
            static_assert(sizeof...(targets) <= lobby_max_players);
        }

        update_target(bool inclusive, std::same_as<player *> auto ... targets)
            : update_target(inclusive, false, targets...) {}

        auto targets() const {
            return std::span{m_targets, m_targets + m_num_targets};
        }

    public:
        static update_target includes(std::same_as<player *> auto ... targets) {
            return update_target(true, targets...);
        }

        static update_target excludes(std::same_as<player *> auto ... targets) {
            return update_target(false, targets...);
        }

        static update_target includes_private(std::same_as<player *> auto ... targets) {
            return update_target(true, true, targets...);
        }

        static update_target excludes_public(std::same_as<player *> auto ... targets) {
            return update_target(false, true, targets...);
        }

        void add(player *target) {
            m_targets[m_num_targets++] = target;
        }

        bool matches(player *target) const {
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
        util::id_map<card> cards;
        util::id_map<player> players;

        card *find_card(int card_id) const {
            if (auto it = cards.find(card_id); it != cards.end()) {
                return &*it;
            }
            throw std::runtime_error(fmt::format("server.find_card: ID {} not found", card_id));
        }

        player *find_player(int player_id) const {
            if (auto it = players.find(player_id); it != players.end()) {
                return &*it;
            }
            throw std::runtime_error(fmt::format("server.find_player: ID {} not found", player_id));
        }
    };

    struct game_update_tuple {
        update_target target;
        json::json content;
        ticks duration;
    };

    struct game_net_manager {
        std::deque<game_update_tuple> m_updates;
        std::deque<std::pair<update_target, game_string>> m_saved_log;

        game_context m_context;

        const game_context &context() const {
            return m_context;
        }

        game_action deserialize_action(const json::json &action) const;
        
        json::json serialize_update(const game_update &update) const;

        template<game_update_type E>
        json::json make_update(auto && ... args) {
            return serialize_update(game_update{enums::enum_tag<E>, FWD(args) ... });
        }

        template<game_update_type E>
        void add_update(update_target target, auto && ... args) {
            game_update update{enums::enum_tag<E>, FWD(args) ... };
            m_updates.emplace_back(target, serialize_update(update), [&]{
                if constexpr (game_update::has_type<E>) {
                    using value_type = enums::enum_type_t<E>;
                    if constexpr (requires { value_type::duration; }) {
                        return std::chrono::duration_cast<ticks>(update.get<E>().duration);
                    }
                }
                return ticks{0};
            }());
        }

        template<game_update_type E>
        void add_update(auto && ... args) {
            add_update<E>(update_target::excludes(), FWD(args) ... );
        }

        template<size_t N, typename ... Ts>
        void add_log(update_target target, const char (&message)[N], Ts && ... args) {
            const auto &log = m_saved_log.emplace_back(target, game_string(message, FWD(args) ...));
            add_update<game_update_type::game_log>(std::move(target), log.second);
        }

        template<size_t N, typename ... Ts>
        void add_log(const char (&message)[N], Ts && ... args) {
            add_log(update_target::excludes(), message, FWD(args) ... );
        }
    };

}

#endif