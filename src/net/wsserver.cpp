#include "wsserver.h"

namespace net {

wsserver::wsserver() {
    m_server.init_asio();

    m_server.set_access_channels(websocketpp::log::alevel::all);
    m_server.clear_access_channels(
        websocketpp::log::alevel::frame_payload |
        websocketpp::log::alevel::frame_header);
}

void wsserver::start(uint16_t port) {
    m_server.set_open_handler([this](client_handle con) {
        std::scoped_lock lock(m_con_mutex);
        m_clients.emplace(con);
        on_connect(con);
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
        on_message(con, msg->get_payload());
    });

    std::error_code ec;
    // m_server.set_reuse_addr(true);
    m_server.listen(port, ec);
    m_server.start_accept(ec);
    if (ec) {
        throw std::system_error(ec);
    }
}

void wsserver::stop() {
    m_server.set_open_handler(nullptr);
    m_server.set_close_handler(nullptr);
    m_server.set_fail_handler(nullptr);
    m_server.set_message_handler(nullptr);

    std::error_code ec;
    m_server.stop_listening(ec);
    for (client_handle hdl : m_clients) {
        m_server.close(hdl, websocketpp::close::status::going_away, "SERVER_STOP", ec);
    }
    m_server.run();
}

void wsserver::tick() {
    m_server.poll();
}

void wsserver::push_message(client_handle con, const std::string &message) {
    if (!con.expired()) {
        std::error_code ec;
        m_server.send(con, message, websocketpp::frame::opcode::text, ec);
    }
}

void wsserver::kick_client(client_handle con, const std::string &msg) {
    if (!con.expired()) {
        on_disconnect(con);
        m_server.close(con, websocketpp::close::status::normal, msg);
    }
}

std::string wsserver::get_client_ip(client_handle con) {
    if (!con.expired()) {
        std::error_code ec;
        auto client_con = m_server.get_con_from_hdl(con, ec);
        if (client_con) {
            std::string header_ip = client_con->get_request_header("X-Real-IP");
            if (!header_ip.empty()) {
                return header_ip;
            } else {
                return client_con->get_remote_endpoint();
            }
        }
    }
    return "(unknown host)";
}

};