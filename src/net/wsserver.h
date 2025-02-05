#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <memory>
#include <vector>

#include "utils/tsqueue.h"

namespace net {

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
        using message_type = std::variant<std::string, connected, disconnected>;
        utils::tsqueue<std::pair<client_handle, message_type>> m_message_queue;

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

        void poll();

        void push_message(client_handle client, std::string message);

        void push_messages(message_list messages);

        void kick_client(client_handle client, std::string message, int code = kick_opcode);
    };

}

#endif