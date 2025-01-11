#ifndef __CHAT_COMMANDS_H__
#define __CHAT_COMMANDS_H__

#include "lobby.h"

namespace banggame {

    enum class command_permissions {
        lobby_owner,
        lobby_waiting,
        lobby_playing,
        lobby_finished,
        lobby_in_game,
        game_cheat,
    };

    template<auto FnMemPtr> struct proxy_t {
        static constexpr auto value = FnMemPtr;
    };

    template<auto FnMemPtr> static constexpr proxy_t<FnMemPtr> proxy;

    template<typename FnMemPtr> struct argument_number;

    template<typename ... Args>
    struct argument_number<void (game_manager::* const)(session_ptr, Args...)> {
        static constexpr size_t value = sizeof...(Args);
    };

    using manager_fn = void (*)(game_manager *, session_ptr, std::span<std::string>);

    class chat_command;

    using string_command_map = std::vector<std::pair<std::string_view, chat_command>>;

    class chat_command {
    public:
        static constexpr char start_char = '/';
        static const string_command_map commands;

    private:
        manager_fn m_fun;
        std::string_view m_description;
        enums::bitset<command_permissions> m_permissions;

        static std::string_view get_arg(std::span<std::string> args, size_t index) {
            if (index < args.size()) {
                return args[index];
            } else {
                return {};
            }
        }

        template<typename Proxy, size_t ... Is>
        static void call_manager_fun_impl(game_manager *mgr, session_ptr session, std::span<std::string> args, std::index_sequence<Is...>) {
            (mgr->*Proxy::value)(session, get_arg(args, Is) ...);
        }

        template<typename Proxy>
        static void call_manager_fun(game_manager *mgr, session_ptr session, std::span<std::string> args) {
            call_manager_fun_impl<Proxy>(mgr, session, args,
                std::make_index_sequence<argument_number<decltype(Proxy::value)>::value>());
        }

    public:
        template<typename Proxy>
        chat_command(Proxy, std::string_view description, enums::bitset<command_permissions> permissions = {})
            : m_fun(call_manager_fun<Proxy>)
            , m_description(description)
            , m_permissions(permissions) {}

        void operator()(game_manager *mgr, session_ptr session, std::span<std::string> args) const {
            (*m_fun)(mgr, session, args);
        }

        std::string_view description() const {
            return m_description;
        }

        enums::bitset<command_permissions> permissions() const {
            return m_permissions;
        }
    };

}

#endif
