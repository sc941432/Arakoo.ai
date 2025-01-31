#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

// Declare any functions or handlers you need to use in tests (e.g., `on_message`)
