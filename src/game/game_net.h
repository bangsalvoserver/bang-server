#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <deque>
#include <numeric>

#include "player.h"
#include "game_update.h"

namespace banggame {

    struct game_context {
        virtual card_ptr find_card(int card_id) const = 0;
        virtual player_ptr find_player(int player_id) const = 0;
        virtual game_duration transform_duration(game_duration duration) const = 0;
    };

    using update_target = player_set;

    struct game_update_record {
        update_target target;
        json::json content;
        game_duration duration;
    };

    struct saved_log_record {
        update_target target;
        json::json content;
    };

    class game_net_manager : public game_context {
    protected:
        std::deque<game_update_record> m_updates;
        std::deque<saved_log_record> m_saved_log;

    private:
        std::unordered_map<int, player_ptr> m_players_by_userid;

    public:
        bool pending_updates() const {
            return !m_updates.empty();
        }

        game_update_record get_next_update() {
            auto update = std::move(m_updates.front());
            m_updates.pop_front();
            return update;
        }
        
        json::json serialize_update(const game_update &update) const;

        void handle_game_action(player_ptr origin, const json::json &value);

    public:
        player_ptr find_player_by_userid(int user_id) const; 

        void update_player_userid(player_ptr target, int user_id);

    public:
        template<std::convertible_to<game_update> Update>
        const game_update_record &add_update(update_target target, Update &&update) {
            game_duration duration{};
            if constexpr (requires { update.duration; }) {
                duration = update.duration.get();
            }
            return m_updates.emplace_back(target, serialize_update(game_update{std::forward<Update>(update)}), duration);
        }

        template<std::convertible_to<game_update> Update>
        const game_update_record &add_update(Update &&update) {
            return add_update(update_target::excludes(), std::forward<Update>(update));
        }

        template<size_t N, typename ... Ts>
        const saved_log_record &add_log(update_target target, const char (&message)[N], Ts && ... args) {
            const auto &update = add_update(target, game_updates::game_log{ game_string(message, std::forward<Ts>(args) ...) });
            return m_saved_log.emplace_back(target, update.content);
        }

        template<size_t N, typename ... Ts>
        const saved_log_record &add_log(const char (&message)[N], Ts && ... args) {
            return add_log(update_target::excludes(), message, std::forward<Ts>(args) ... );
        }
    };

}

#endif