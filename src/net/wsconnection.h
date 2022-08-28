#ifndef __WSCONNECTION_H__
#define __WSCONNECTION_H__

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "utils/json_serial.h"

namespace net {

    template<typename Derived, typename InputMessage, typename OutputMessage>
    class wsconnection {
    public:
        using client_type = websocketpp::client<websocketpp::config::asio_client>;
        using client_handle = websocketpp::connection_hdl;

    private:
        client_type m_client;
        client_type::connection_weak_ptr m_con;

        std::string m_address;

        bool check_client_handle(client_handle hdl) const {
            return !hdl.owner_before(m_con) && !m_con.owner_before(hdl);
        }

    public:
        wsconnection(asio::io_context &ctx) {
            m_client.init_asio(&ctx);
            m_client.set_access_channels(websocketpp::log::alevel::none);
            m_client.set_error_channels(websocketpp::log::alevel::none);
        }
            
        void connect(const std::string &url) {
            m_client.set_open_handler([this](client_handle hdl) {
                if (check_client_handle(hdl)) {
                    if constexpr (requires (Derived obj) { obj.on_open(); }) {
                        static_cast<Derived &>(*this).on_open();
                    }
                }
            });

            auto handler = [this](client_handle hdl) {
                if (check_client_handle(hdl)) {
                    if constexpr (requires (Derived obj) { obj.on_close(); }) {
                        static_cast<Derived &>(*this).on_close();
                    }
                    m_con.reset();
                }
            };

            m_client.set_close_handler(handler);
            m_client.set_fail_handler(handler);

            m_client.set_message_handler([this](client_handle hdl, client_type::message_ptr msg) {
                if (check_client_handle(hdl)) {
                    try {
                        std::stringstream ss(msg->get_payload());
                        Json::Value json_value;
                        ss >> json_value;
                        static_cast<Derived &>(*this).on_message(json::deserialize<InputMessage>(json_value));
                    } catch (const std::exception &) {
                        disconnect();
                    }
                }
            });

            std::error_code ec;
            m_address = url;
            auto con = m_client.get_connection(std::string("ws://") + url, ec);
            if (!ec) {
                m_client.connect(con);
                m_con = con;
            }
        }
        
        void disconnect() {
            if (auto con = m_con.lock()) {
                switch (con->get_state()) {
                case websocketpp::session::state::connecting:
                    con->terminate(make_error_code(websocketpp::error::operation_canceled));
                    break;
                case websocketpp::session::state::open:
                    con->close(0, "");
                    break;
                }
            }
        }

        const std::string &address_string() const {
            return m_address;
        }

        void push_message(auto && ... args) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::error_code ec;
            m_client.send(m_con, Json::writeString(builder, json::serialize(OutputMessage{FWD(args) ... })),
                websocketpp::frame::opcode::text, ec);
        }
    };
}

#endif