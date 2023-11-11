#ifndef __WSSERVER_H__
#define __WSSERVER_H__

#include <sstream>
#include <set>

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace net {

class wsserver {
public:
    using server_type = websocketpp::server<websocketpp::config::asio>;
    using client_handle = websocketpp::connection_hdl;

private:
    server_type m_server;

    std::set<client_handle, std::owner_less<client_handle>> m_clients;
    std::mutex m_con_mutex;

protected:
    virtual void on_disconnect(client_handle handle) = 0;
    virtual void on_message(client_handle hdl, const std::string &message) = 0;

public:
    wsserver(asio::io_context &ctx);

    virtual ~wsserver();

    bool start(uint16_t port);

    void push_message(client_handle con, const std::string &message);

    void kick_client(client_handle con, const std::string &msg);

    std::string get_client_ip(client_handle con);

};

}

#endif