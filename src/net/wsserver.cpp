#include "wsserver.h"

namespace net {

wsserver::wsserver(asio::io_context &ctx) {
    m_server.init_asio(&ctx);

    m_server.set_access_channels(websocketpp::log::alevel::all);
    m_server.clear_access_channels(
        websocketpp::log::alevel::frame_payload |
        websocketpp::log::alevel::frame_header);
}

wsserver::~wsserver() {
    if (m_server.is_listening()) {
        m_server.stop_listening();
    }
}

bool wsserver::start(uint16_t port) {
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
        on_message(con, msg->get_payload());
    });

    std::error_code ec;
    m_server.listen(port, ec);
    m_server.start_accept(ec);
    return !ec;
}

void wsserver::push_message(client_handle con, const std::string &message) {
    std::error_code ec;
    m_server.send(con, message, websocketpp::frame::opcode::text, ec);
}

void wsserver::kick_client(client_handle con, const std::string &msg) {
    m_server.close(con, 0, msg);
}

std::string wsserver::get_client_ip(client_handle con) {
    std::error_code ec;
    auto client_con = m_server.get_con_from_hdl(con, ec);
    if (client_con) {
        return client_con->get_remote_endpoint();
    }
    return "(unknown host)";
}

};