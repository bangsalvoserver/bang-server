#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <sstream>
#include <set>

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "logging.h"

namespace net {

    using log_level_fn_ptr = logging::level (*)(websocketpp::log::level);

    template<log_level_fn_ptr LogLevelFunction>
    struct logging_adapter {
        logging_adapter(websocketpp::log::level, websocketpp::log::channel_type_hint::value) {}
            
        void set_channels(websocketpp::log::level) {}
        void clear_channels(websocketpp::log::level) {}
        constexpr bool static_test(websocketpp::log::level) const { return true; }
        bool dynamic_test(websocketpp::log::level) { return true; }
        
        void write(websocketpp::log::level channel, std::string_view msg) {
            logging::log_function(LogLevelFunction(channel))(msg);
        }
    };

    logging::level get_access_logging_level(websocketpp::log::level channel);
    using access_logging_adapter = logging_adapter<get_access_logging_level>;

    logging::level get_error_logging_level(websocketpp::log::level channel);
    using error_logging_adapter = logging_adapter<get_error_logging_level>;

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