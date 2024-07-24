#include "wsserver.h"

#include "logging.h"

namespace net {

logging::level get_access_logging_level(websocketpp::log::level channel) {
    using alevel = websocketpp::log::alevel;
    switch (channel) {
    case alevel::message_header:
    case alevel::message_payload:
    case alevel::endpoint:
    case alevel::devel:
    case alevel::frame_payload:
    case alevel::frame_header:
        return logging::level::trace;
    case alevel::debug_handshake:
    case alevel::debug_close:
        return logging::level::debug;
    case alevel::connect:
    case alevel::disconnect:
    case alevel::http:
        return logging::level::status;
    case alevel::fail:
        return logging::level::warning;
    default:
        return logging::level::info;
    }
}

logging::level get_error_logging_level(websocketpp::log::level channel) {
    using elevel = websocketpp::log::elevel;
    switch (channel) {
    case elevel::devel:
        return logging::level::trace;
    case elevel::library:
        return logging::level::debug;
    case elevel::info:
        return logging::level::info;
    case elevel::warn:
        return logging::level::warning;
    default:
        return logging::level::error;
    }
}

void wsserver::start(uint16_t port, bool reuse_addr) {
    m_server.init_asio();

    m_server.set_open_handler([this](client_handle con) {
        std::scoped_lock lock(m_con_mutex);
        m_clients.emplace(con);
        on_connect(con);
    });

    auto client_disconnect_handler = [this](client_handle con) {
        std::scoped_lock lock(m_con_mutex);
        if (auto it = m_clients.find(con); it != m_clients.end()) {
            m_clients.erase(it);
            on_disconnect(con);
        }
    };

    m_server.set_close_handler(client_disconnect_handler);
    m_server.set_fail_handler(client_disconnect_handler);

    m_server.set_message_handler([this](client_handle con, server_type::message_ptr msg) {
        on_message(con, msg->get_payload());
    });

    m_server.set_reuse_addr(reuse_addr);

    std::error_code ec;
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