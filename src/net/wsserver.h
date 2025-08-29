#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <memory>
#include <vector>
#include <variant>
#include <chrono>

#include "utils/tsqueue.h"

namespace net {

    using namespace std::chrono_literals;

    static constexpr size_t max_message_log_size = 1000;

    class wsserver_impl;

    struct wsserver_impl_deleter {
        void operator()(wsserver_impl *ptr) const;
    };

    class wsserver {
    public:
        using client_handle = std::weak_ptr<void>;
        using message_list = std::vector<std::pair<client_handle, std::string>>;

        static constexpr int kick_opcode = 1000;

    private:
        std::unique_ptr<wsserver_impl, wsserver_impl_deleter> m_server;

        struct connected {};
        struct disconnected {};
        using message_type = std::pair<client_handle, std::variant<std::string, connected, disconnected>>;
        utils::tsqueue<message_type> m_message_queue;

        void invoke_message(message_type message);

    protected:
        virtual void on_connect(client_handle handle) = 0;
        virtual void on_disconnect(client_handle handle) = 0;
        virtual void on_message(client_handle hdl, std::string_view message) = 0;

    public:
        virtual ~wsserver() = default;

        void init();
#ifndef LIBUS_NO_SSL
        void init_tls(const std::string &certificate_file, const std::string &private_key_file);
#endif

        void start(uint16_t port, bool reuse_addr = false);

        void stop();

        void poll() {
            while (auto message = m_message_queue.pop()) {
                invoke_message(*message);
            }
        }

        template<typename Rep, typename Period>
        void wait(std::chrono::duration<Rep, Period> rel_time = 1s) {
            if (auto message = m_message_queue.wait(rel_time)) {
                invoke_message(*message);
            }
        }

        void push_message(client_handle client, std::string message);

        void push_messages(message_list messages);

        void kick_client(client_handle client, std::string message, int code = kick_opcode);
    };

}

#endif