#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <numeric>

#include "player.h"
#include "game_update.h"

namespace banggame {

    class update_target {
    private:
        uint16_t m_value = 0;

        update_target(uint16_t value)
            : m_value{value} {}

        update_target(bool exclusive, std::convertible_to<const_player_ptr> auto ... targets)
            : m_value{static_cast<uint16_t>(exclusive)}
        {
            (add(targets), ...);
        }

    public:
        update_target() = default;

        static update_target includes(std::convertible_to<const_player_ptr> auto ... targets) {
            return update_target(false, targets...);
        }

        static update_target excludes(std::convertible_to<const_player_ptr> auto ... targets) {
            return update_target(true, targets...);
        }

        void add(const_player_ptr target) {
            m_value |= 1 << target->id;
        }

        bool contains(const_player_ptr target) const {
            return target && bool(m_value & (1 << target->id));
        }

        bool matches(const_player_ptr target) const {
            return contains(target) != exclusive();
        }

        bool exclusive() const {
            return bool(m_value & 1);
        }

        explicit operator bool() const {
            return m_value != 0;
        }

        update_target operator ~() const {
            if (exclusive()) {
                return update_target{static_cast<uint16_t>(m_value & ~1)};
            } else {
                return update_target{static_cast<uint16_t>(m_value | 1)};
            }
        }

        update_target operator - (const update_target &rhs) const {
            return update_target{static_cast<uint16_t>(m_value & ~rhs.m_value)};
        }
    };

    struct game_context {
        virtual card_ptr find_card(int card_id) const = 0;
        virtual player_ptr find_player(int player_id) const = 0;
        virtual player_ptr find_player_by_userid(int user_id) const = 0;
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

    public:
        bool pending_updates() const {
            return !m_updates.empty();
        }

        game_update_tuple get_next_update() {
            auto update = std::move(m_updates.front());
            m_updates.pop_front();
            return update;
        }
        
        json::json serialize_update(const game_update &update) const;

        void handle_game_action(player_ptr origin, const json::json &value);

    public:
        template<std::convertible_to<game_update> Update>
        void add_update(update_target target, Update &&update) {
            game_duration duration{};
            if constexpr (requires { update.duration; }) {
                duration = update.duration.get();
            }
            m_updates.emplace_back(target, serialize_update(game_update{std::forward<Update>(update)}), duration);
        }

        template<std::convertible_to<game_update> Update>
        void add_update(Update &&update) {
            add_update(update_target::excludes(), std::forward<Update>(update));
        }

        template<size_t N, typename ... Ts>
        void add_log(update_target target, const char (&message)[N], Ts && ... args) {
            const auto &log = m_saved_log.emplace_back(target, game_string(message, FWD(args) ...));
            add_update(target, game_updates::game_log{ log.second });
        }

        template<size_t N, typename ... Ts>
        void add_log(const char (&message)[N], Ts && ... args) {
            add_log(update_target::excludes(), message, FWD(args) ... );
        }
    };

}

#endif