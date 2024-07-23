#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <sstream>
#include <set>

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "logging.h"

namespace net {

    template<typename Derived>
    struct logging_adapter {
        const websocketpp::log::level m_static_channels;
        websocketpp::log::level m_dynamic_channels;

        logging_adapter(websocketpp::log::level channels, websocketpp::log::channel_type_hint::value)
            : m_static_channels(channels)
            , m_dynamic_channels(0) {}
            
        void set_channels(websocketpp::log::level channels) {
            if (channels == 0) {
                m_dynamic_channels = 0;
            } else {
                m_dynamic_channels |= (channels & m_static_channels);
            }
        }

        void clear_channels(websocketpp::log::level channels) {
            m_dynamic_channels &= ~channels;
        }

        constexpr bool static_test(websocketpp::log::level channel) const {
            return ((channel & m_static_channels) != 0);
        }

        bool dynamic_test(websocketpp::log::level channel) {
            return ((channel & m_dynamic_channels) != 0);
        }
        
        void write(websocketpp::log::level channel, const std::string &msg) {
            if (dynamic_test(channel)) {
                logging::log_function(Derived::get_logging_level(channel))("{}", msg);
            }
        }
    };

    struct access_logging_adapter : logging_adapter<access_logging_adapter> {
        using logging_adapter<access_logging_adapter>::logging_adapter;
        static logging::level get_logging_level(websocketpp::log::level channel) {
            return logging::level::info;
        }
    };

    struct error_logging_adapter : logging_adapter<error_logging_adapter> {
        using logging_adapter<error_logging_adapter>::logging_adapter;
        static logging::level get_logging_level(websocketpp::log::level channel) {
            return logging::level::error;
        }
    };

    using wsconfig_base = websocketpp::config::asio;
    struct wsconfig : wsconfig_base {
        using alog_type = access_logging_adapter;
        using elog_type = error_logging_adapter;

        struct transport_config : wsconfig_base::transport_config {
            using alog_type = access_logging_adapter;
            using elog_type = error_logging_adapter;
        };

        using transport_type = websocketpp::transport::asio::endpoint<transport_config>;
    };

    class wsserver {
    public:
        using server_type = websocketpp::server<wsconfig>;
        using client_handle = websocketpp::connection_hdl;

    private:
        server_type m_server;

        std::set<client_handle, std::owner_less<client_handle>> m_clients;
        std::mutex m_con_mutex;

    protected:
        virtual void on_connect(client_handle handle) = 0;
        virtual void on_disconnect(client_handle handle) = 0;
        virtual void on_message(client_handle hdl, const std::string &message) = 0;

    public:
        virtual ~wsserver() = default;

        void start(uint16_t port, bool reuse_addr = false);

        void stop();

        void tick();

        void push_message(client_handle con, const std::string &message);

        void kick_client(client_handle con, const std::string &msg);

        std::string get_client_ip(client_handle con);

    };

}

#endif