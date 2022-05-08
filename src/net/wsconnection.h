#ifndef __WSCONNECTION_H__
#define __WSCONNECTION_H__

#include <boost/asio.hpp>

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

    public:
        wsconnection(boost::asio::io_context &ctx) {
            m_client.init_asio(&ctx);
            m_client.set_access_channels(websocketpp::log::alevel::none);
            m_client.set_error_channels(websocketpp::log::alevel::none);
        }
            
        void connect(const std::string &url) {
            if constexpr (requires (Derived obj) { obj.on_open(); }) {
                m_client.set_open_handler([this](client_handle) {
                    static_cast<Derived &>(*this).on_open();
                });
            }

            if constexpr (requires (Derived obj) { obj.on_close(); }) {
                auto handler = [this](client_handle) {
                    static_cast<Derived &>(*this).on_close();
                };

                m_client.set_close_handler(handler);
                m_client.set_fail_handler(handler);
            }

            m_client.set_message_handler([this](client_handle, client_type::message_ptr msg) {
                try {
                    std::stringstream ss(msg->get_payload());
                    Json::Value json_value;
                    ss >> json_value;
                    static_cast<Derived &>(*this).on_message(json::deserialize<InputMessage>(json_value));
                } catch (const std::exception &e) {
                    // ignore
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
            std::error_code ec;
            if (!m_con.expired()) {
                m_client.close(m_con, 0, "", ec);
            }
            if constexpr (requires (Derived obj) { obj.on_close(); }) {
                if (ec) {
                    static_cast<Derived &>(*this).on_close();
                }
            }
        }

        const std::string &address_string() const {
            return m_address;
        }

        template<typename ... Ts> void push_message(Ts && ... args) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::error_code ec;
            m_client.send(m_con, Json::writeString(builder, json::serialize(OutputMessage{std::forward<Ts>(args) ... })),
                websocketpp::frame::opcode::text, ec);
        }
    };
}

#endif