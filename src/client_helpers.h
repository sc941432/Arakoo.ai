#ifndef CLIENT_HELPERS_H
#define CLIENT_HELPERS_H

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/ssl/context.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

extern std::mutex mtx;
extern std::condition_variable cv;
extern bool connected;

// Modify handlers to match WebSocket++'s expected signatures
void on_open(websocketpp::connection_hdl hdl);
void on_message(websocketpp::connection_hdl, client::message_ptr);
void on_fail(websocketpp::connection_hdl);
void on_close(websocketpp::connection_hdl);
context_ptr on_tls_init(websocketpp::connection_hdl);

#endif // CLIENT_HELPERS_H
