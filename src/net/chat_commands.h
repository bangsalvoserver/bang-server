#ifndef __CHAT_COMMANDS_H__
#define __CHAT_COMMANDS_H__

#include "lobby.h"

namespace banggame {

    DEFINE_ENUM_FLAGS(command_permissions,
        (lobby_owner)
        (lobby_waiting)
        (lobby_playing)
        (lobby_finished)
        (game_cheat)
    )

    template<auto FnMemPtr> struct proxy_t {
        static constexpr auto value = FnMemPtr;
    };

    template<auto FnMemPtr> static constexpr proxy_t<FnMemPtr> proxy;

    template<typename FnMemPtr> struct argument_number;

    template<typename ... Args>
    struct argument_number<std::string (game_manager::* const)(user_ptr, Args...)> {
        static constexpr size_t value = sizeof...(Args);
    };

    using manager_fn = std::string (*)(game_manager *, user_ptr, std::span<std::string>);

    class chat_command {
    public:
        static constexpr char start_char = '/';
        static const std::map<std::string, chat_command, std::less<>> commands;

    private:
        manager_fn m_fun;
        std::string_view m_description;
        command_permissions m_permissions;

        static std::string_view get_arg(std::span<std::string> args, size_t index) {
            if (index < args.size()) {
                return args[index];
            } else {
                return {};
            }
        }

        template<typename Proxy, size_t ... Is>
        static std::string call_manager_fun_impl(game_manager *mgr, user_ptr user, std::span<std::string> args, std::index_sequence<Is...>) {
            return (mgr->*Proxy::value)(user, get_arg(args, Is) ...);
        }

        template<typename Proxy>
        static std::string call_manager_fun(game_manager *mgr, user_ptr user, std::span<std::string> args) {
            return call_manager_fun_impl<Proxy>(mgr, user, args,
                std::make_index_sequence<argument_number<decltype(Proxy::value)>::value>());
        }

    public:
        template<typename Proxy>
        chat_command(Proxy, std::string_view description, command_permissions permissions = {})
            : m_fun(call_manager_fun<Proxy>)
            , m_description(description)
            , m_permissions(permissions) {}

        std::string operator()(game_manager *mgr, user_ptr user, std::span<std::string> args) const {
            return (*m_fun)(mgr, user, args);
        }

        std::string_view description() const {
            return m_description;
        }

        command_permissions permissions() const {
            return m_permissions;
        }
    };

}

#endif
