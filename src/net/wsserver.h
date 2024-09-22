#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <memory>
#include <variant>
#include <stdexcept>
#include <deque>

#include <App.h>

namespace net {

    class wsserver {
    public:
        using client_handle = std::weak_ptr<void>;

    private:
        std::variant<
            std::monostate,
            uWS::App
#ifndef LIBUS_NO_SSL
            , uWS::SSLApp
#endif
        > m_server;

        std::deque<std::pair<client_handle, std::variant<std::string, bool>>> m_message_queue;
        std::mutex m_message_mutex;

        uWS::Loop *m_loop = nullptr;
        us_listen_socket_t *m_listen_socket = nullptr;

    protected:
        virtual void on_connect(client_handle handle) = 0;
        virtual void on_disconnect(client_handle handle) = 0;
        virtual void on_message(client_handle hdl, std::string_view message) = 0;
        virtual void kick_all_clients() = 0;

    public:
        virtual ~wsserver() = default;

        void init();
#ifndef LIBUS_NO_SSL
        void init_tls(const std::string &certificate_file, const std::string &private_key_file);
#endif

        void start(uint16_t port, bool reuse_addr = false);

        void stop();

        void tick();

        void push_message(client_handle con, const std::string &message);

        void kick_client(client_handle con, const std::string &msg);

        std::string get_client_ip(client_handle con);
    };

}

#endif