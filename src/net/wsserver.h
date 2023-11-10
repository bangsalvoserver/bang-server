#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <sstream>
#include <set>

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "utils/json_serial.h"

namespace net {

template<typename Derived, typename InputMessage, typename OutputMessage>
class wsserver {
public:
    using server_type = websocketpp::server<websocketpp::config::asio>;
    using client_handle = websocketpp::connection_hdl;

private:
    server_type m_server;

    std::set<client_handle, std::owner_less<client_handle>> m_clients;
    std::mutex m_con_mutex;

    void on_disconnect(client_handle handle) {
        if constexpr (requires (Derived obj) { obj.on_disconnect(handle); }) {
            static_cast<Derived &>(*this).on_disconnect(handle);
        }
    }

public:
    wsserver(asio::io_context &ctx) {
        m_server.init_asio(&ctx);

        m_server.set_access_channels(websocketpp::log::alevel::all);
        m_server.clear_access_channels(
            websocketpp::log::alevel::frame_payload |
            websocketpp::log::alevel::frame_header);
    }

    ~wsserver() {
        if (m_server.is_listening()) {
            m_server.stop_listening();
        }
    }

    bool start(uint16_t port) {
        m_server.set_open_handler([this](client_handle con) {
            std::scoped_lock lock(m_con_mutex);
            m_clients.emplace(con);
        });

        auto client_disconnect_handler = [this](client_handle con) {
            std::scoped_lock lock(m_con_mutex);
            std::erase_if(m_clients, [&](client_handle c) {
                if (auto ptr = c.lock()) {
                    if (con.lock().get() == ptr.get()) {
                        on_disconnect(c);
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    on_disconnect(c);
                    return true;
                }
            });
        };

        m_server.set_close_handler(client_disconnect_handler);
        m_server.set_fail_handler(client_disconnect_handler);

        m_server.set_message_handler([this](client_handle con, server_type::message_ptr msg) {
            try {
                static_cast<Derived &>(*this).on_message(con, json::deserialize<InputMessage>(
                    json::json::parse(msg->get_payload())));
            } catch (const std::exception &) {
                m_server.close(con, 0, "");
            }
        });

        std::error_code ec;
        m_server.listen(port, ec);
        m_server.start_accept(ec);
        return !ec;
    }

    void push_message(client_handle con, const OutputMessage &msg) {
        std::error_code ec;
        m_server.send(con, json::serialize(msg).dump(),
            websocketpp::frame::opcode::text, ec);
    }

    void kick_client(client_handle con, const std::string &msg) {
        m_server.close(con, 0, msg);
    }

    std::string get_client_ip(client_handle con) {
        std::error_code ec;
        auto client_con = m_server.get_con_from_hdl(con, ec);
        if (client_con) {
            return fmt::format("{}:{}", client_con->get_host(), client_con->get_port());
        }
        return "(unknown host)";
    }

};

}

#endif