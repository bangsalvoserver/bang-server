#ifndef __WSBANG_H__
#define __WSBANG_H__

#include "wsserver.h"
#include "wsconnection.h"

#include "manager.h"
#include "options.h"

#include <iostream>
#include <thread>

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

            m_mgr.set_kick_client_function([&](client_handle con, const std::string &msg) {
                this->kick_client(con, msg);
            });

            m_mgr.set_client_ip_function([&](client_handle con) {
                return this->get_client_ip(con);
            });
        }

        void tick() {
            m_mgr.tick();
        }

        void on_disconnect(client_handle con) {
            m_mgr.client_disconnected(con);
        }

        void on_message(client_handle con, const client_message &msg) {
            m_mgr.on_receive_message(con, msg);
        }
    };

}

#endif