#ifndef __WSBANG_H__
#define __WSBANG_H__

#include "wsserver.h"
#include "wsconnection.h"

#include "game/manager.h"
#include "game/net_options.h"

namespace banggame {

    struct bang_server : net::wsserver<bang_server, client_message, server_message> {
        using base = net::wsserver<bang_server, client_message, server_message>;
        using client_handle = typename base::client_handle;

        game_manager m_mgr;
        std::jthread m_game_thread;

        bang_server(asio::io_context &ctx) : base(ctx) {
            m_mgr.set_send_message_function([&](client_handle con, banggame::server_message msg) {
                this->push_message(con, std::move(msg));
            });
        }

        bool start(uint16_t port = default_server_port) {
            if (!base::start(port)) return false;
            m_game_thread = std::jthread(std::bind_front(&game_manager::start, &m_mgr));
            return true;
        }

        void on_disconnect(client_handle con) {
            m_mgr.client_disconnected(con);
        }

        void on_message(client_handle con, client_message msg) {
            m_mgr.on_receive_message(con, std::move(msg));
        }
    };

}

#endif