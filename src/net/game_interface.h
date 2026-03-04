#ifndef __GAME_INTERFACE_H__
#define __GAME_INTERFACE_H__

#include <functional>
#include <string>
#include <span>

#include "utils/json_serial.h"
#include "utils/function_ref.h"

namespace banggame {

    struct game_lobby;

    using string_span = std::span<std::string>;

    class chat_command {
    private:
        std::string_view m_name;
        std::string_view m_description;
        std::move_only_function<void(game_lobby &lobby, int user_id, string_span args) const> m_fun;

    public:
        static constexpr char start_char = '/';

        template<typename Function>
        chat_command(std::string_view name, std::string_view description, Function &&fun)
            : m_name{name}
            , m_description{description}
            , m_fun{[fun=std::move(fun)](game_lobby &lobby, int user_id, string_span args) {
                [&]<size_t ... I>(std::index_sequence<I ...>) {
                    std::invoke(fun, lobby, user_id, (I < args.size() ? args[I] : std::string{}) ...);
                }(std::make_index_sequence<argument_number_v<std::remove_cvref_t<Function>> - 2>());
            }} {}
        
        std::string_view name() const {
            return m_name;
        }

        std::string_view description() const {
            return m_description;
        }

        void operator()(game_lobby &lobby, int user_id, string_span args) const {
            m_fun(lobby, user_id, args);
        }
    };

    using update_content = json::raw_string;

    template<typename ... Ts>
    using consumer_callback = utils::function_ref<void(Ts...)>;

    struct game_interface {
        virtual ~game_interface() = default;

        virtual void tick() = 0;
        virtual void get_pending_updates(std::span<const int> user_ids, consumer_callback<int, update_content> callback) = 0;
        virtual void get_spectator_join_updates(consumer_callback<update_content> callback) = 0;
        virtual void get_rejoin_updates(int user_id, consumer_callback<update_content> callback) = 0;
        virtual void handle_game_action(int user_id, const json::json &action) = 0;
        virtual void rejoin_user(int old_user_id, int new_user_id) = 0;
        virtual void start_game(std::span<int> user_ids) = 0;
        virtual bool is_game_over() const = 0;
        virtual void get_game_commands(bool enable_cheats, consumer_callback<chat_command> callback) const = 0;
    };
}

#endif