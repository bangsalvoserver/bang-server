#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <numeric>

#include "player.h"
#include "game_update.h"

namespace banggame {

    class update_target {
    private:
        uint16_t m_value;

        update_target(bool inclusive, std::convertible_to<const_player_ptr> auto ... targets)
            : m_value{static_cast<uint16_t>(inclusive)}
        {
            (add(targets), ...);
        }

    public:
        static update_target includes(std::convertible_to<const_player_ptr> auto ... targets) {
            return update_target(true, targets...);
        }

        static update_target excludes(std::convertible_to<const_player_ptr> auto ... targets) {
            return update_target(false, targets...);
        }

        void add(const_player_ptr target) {
            m_value |= 1 << target->id;
        }

        bool contains(const_player_ptr target) const {
            return target && bool(m_value & (1 << target->id));
        }

        bool matches(const_player_ptr target) const {
            return contains(target) == inclusive();
        }

        bool inclusive() const {
            return bool(m_value & 1);
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

    private:
        json::json serialize_update(const game_update &update) const;

    protected:
        template<utils::fixed_string E> requires game_update_type<E>
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

        void handle_game_action(player_ptr origin, const json::json &value);

    public:
        template<utils::fixed_string E> requires game_update_type<E>
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

        template<utils::fixed_string E> requires game_update_type<E>
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