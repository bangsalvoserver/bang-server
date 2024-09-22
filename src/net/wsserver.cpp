#include "wsserver.h"

#include "logging.h"

namespace net {

    struct wsclient_data {
        std::shared_ptr<void *> client;
        std::string address;
    };

    void wsserver::init() {
        m_server.emplace<uWS::App>();
    }

#ifndef LIBUS_NO_SSL
    void wsserver::init_tls(const std::string &certificate_file, const std::string &private_key_file) {
        m_server.emplace<uWS::SSLApp>(uWS::SocketContextOptions{
            .key_file_name = private_key_file.c_str(),
            .cert_file_name = certificate_file.c_str()
        });
    }
#endif

    template<typename Function, typename Server>
    void visit_server(Function &&fn, Server &&server) {
        struct server_visitor : Function {
            using Function::operator();
            void operator()(std::monostate) {};
        };

        std::visit(server_visitor{std::forward<Function>(fn)}, std::forward<Server>(server));
    }

    template<bool SSL>
    uWS::WebSocket<SSL, true, wsclient_data> *websocket_cast(wsserver::client_handle con) {
        if (auto ptr = con.lock()) {
            return *std::static_pointer_cast<uWS::WebSocket<SSL, true, wsclient_data> *>(ptr);
        }
        return nullptr;
    }

    void wsserver::start(uint16_t port, bool reuse_addr) {
        m_loop = uWS::Loop::get();

        int listen_options = reuse_addr ? LIBUS_LISTEN_DEFAULT : LIBUS_LISTEN_EXCLUSIVE_PORT;
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            server.template ws<wsclient_data>("/", {
                .maxPayloadLength = 1 * 1024 * 1024,
                .maxBackpressure = 16 * 1024 * 1024,

                .open = [this](auto *ws) {
                    std::scoped_lock lock{m_message_mutex};
                    wsclient_data *data = ws->getUserData();
                    logging::status("{}: Connected", data->address = ws->getRemoteAddressAsText());
                    m_message_queue.emplace_back(data->client = std::make_shared<void *>(ws), true);
                },
                .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    std::scoped_lock lock{m_message_mutex};
                    wsclient_data *data = ws->getUserData();
                    logging::info("{}: Received {}", data->address, message);
                    m_message_queue.emplace_back(data->client, std::string(message));
                },
                .close = [this](auto *ws, int code, std::string_view message) {
                    std::scoped_lock lock{m_message_mutex};
                    wsclient_data *data = ws->getUserData();
                    logging::status("{}: Disconnected", data->address);
                    m_message_queue.emplace_back(data->client, false);
                }
            }).listen(port, listen_options, [=, this](us_listen_socket_t *listen_socket) {
                m_listen_socket = listen_socket;
                if (listen_socket) {
                    logging::status("Server listening on port {}", port);
                } else {
                    throw std::runtime_error(std::format("Failed to listen on port {}", port));
                }
            }).run();
        }, m_server);
    }

    void wsserver::stop() {
        kick_all_clients();
        
        us_listen_socket_close(0, m_listen_socket);
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            m_loop->defer([&]{
                server.close();
            });
        }, m_server);
    }

    template<typename T>
    static std::optional<T> threadsafe_pop(std::deque<T> &queue, std::mutex &mutex) {
        std::scoped_lock lock{mutex};
        if (queue.empty()) {
            return std::nullopt;
        }
        std::optional<T> result{std::move(queue.front())};
        queue.pop_front();
        return result;
    }

    void wsserver::tick() {
        while (auto elem = threadsafe_pop(m_message_queue, m_message_mutex)) {
            auto [client, message] = *elem;

            std::visit([&](const auto &value){
                if constexpr (std::is_convertible_v<decltype(value), std::string_view>) {
                    on_message(client, value);
                } else {
                    if (value) {
                        on_connect(client);
                    } else {
                        on_disconnect(client);
                    }
                }
            }, message);
        }
    }

    void wsserver::push_message(client_handle con, const std::string &message) {
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            if (auto *ws = websocket_cast<SSL>(con)) {
                m_loop->defer([=]{
                    ws->send(message, uWS::TEXT);
                });
            }
        }, m_server);
    }

    void wsserver::kick_client(client_handle con, const std::string &msg) {
        on_disconnect(con);
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            if (auto *ws = websocket_cast<SSL>(con)) {
                m_loop->defer([=]{
                    ws->end(1, msg);
                });
            }
        }, m_server);
    }

    std::string wsserver::get_client_ip(client_handle con) {
        std::string result = "(unknown host)";
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            if (auto *ws = websocket_cast<SSL>(con)) {
                result = ws->getUserData()->address;
            }
        }, m_server);
        return result;
    }

}