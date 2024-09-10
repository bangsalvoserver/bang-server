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
    case elevel::warn:
        return logging::level::warning;
    default:
        return logging::level::error;
    }
}

void wsserver::init() {
    server_type &server = m_server.emplace<server_type>();
    
    std::error_code ec;
    server.init_asio(ec);

    if (ec) {
        throw std::system_error(ec);
    }
}

#ifndef WSSERVER_NO_TLS
void wsserver::init_tls(const std::string &certificate_file, const std::string &private_key_file) {
    server_type_tls &server = m_server.emplace<server_type_tls>();

    std::error_code ec;
    server.init_asio(ec);

    if (ec) {
        throw std::system_error(ec);
    }
    
    server.set_tls_init_handler([&](client_handle con) {
        auto ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_server);

        ctx->use_certificate_chain_file(certificate_file);
        ctx->use_private_key_file(private_key_file, asio::ssl::context::pem);

        return ctx;
    });
}
#endif

void wsserver::start(uint16_t port, bool reuse_addr) {
    std::visit([&]<typename ServerType>(ServerType &server) {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            server.set_open_handler([this](client_handle con) {
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

            server.set_close_handler(client_disconnect_handler);
            server.set_fail_handler(client_disconnect_handler);

            server.set_message_handler([this](client_handle con, ServerType::message_ptr msg) {
                on_message(con, msg->get_payload());
            });

            server.set_reuse_addr(reuse_addr);

            std::error_code ec;
            server.listen(port, ec);
            server.start_accept(ec);
            if (ec) {
                throw std::system_error(ec);
            }
        }
    }, m_server);
}

void wsserver::stop() {
    std::visit([&]<typename ServerType>(ServerType &server) {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            server.set_open_handler(nullptr);
            server.set_close_handler(nullptr);
            server.set_fail_handler(nullptr);
            server.set_message_handler(nullptr);

            std::error_code ec;
            server.stop_listening(ec);
            for (client_handle hdl : m_clients) {
                server.close(hdl, websocketpp::close::status::going_away, "SERVER_STOP", ec);
            }
            server.run();
        }
    }, m_server);
}

void wsserver::tick() {
    std::visit([&]<typename ServerType>(ServerType &server) {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            server.poll();
        }
    }, m_server);
}

void wsserver::push_message(client_handle con, const std::string &message) {
    std::visit([&]<typename ServerType>(ServerType &server) {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            if (!con.expired()) {
                std::error_code ec;
                server.send(con, message, websocketpp::frame::opcode::text, ec);
            }
        }
    }, m_server);
}

void wsserver::kick_client(client_handle con, const std::string &msg) {
    std::visit([&]<typename ServerType>(ServerType &server) {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            if (!con.expired()) {
                on_disconnect(con);
                server.close(con, websocketpp::close::status::normal, msg);
            }
        }
    }, m_server);
}

std::string wsserver::get_client_ip(client_handle con) {
    return std::visit([&]<typename ServerType>(ServerType &server) -> std::string {
        if constexpr (!std::is_same_v<ServerType, std::monostate>) {
            if (!con.expired()) {
                std::error_code ec;
                auto client_con = server.get_con_from_hdl(con, ec);
                if (!ec && client_con) {
                    return client_con->get_remote_endpoint();
                }
            }
        }
        return "(unknown host)";
    }, m_server);
}

};