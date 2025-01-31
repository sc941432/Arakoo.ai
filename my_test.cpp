#include <gtest/gtest.h>
#include <websocketpp/config/asio_client.hpp>
#include "C:/Users/sc941/OneDrive/Desktop/Arakoo.ai/vcpkg/installed/x64-windows/include/websocketpp/config/asio_no_tls_client.hpp"
#include <websocketpp/client.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

using client = websocketpp::client<websocketpp::config::asio_tls_client>;

std::mutex mtx;
std::condition_variable cv;
bool connected = false;

// TLS initialization handler
auto on_tls_init = [](websocketpp::connection_hdl hdl) -> std::shared_ptr<boost::asio::ssl::context> {
    std::cout << "[DEBUG] TLS init handler called" << std::endl;
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::no_tlsv1 |
                         boost::asio::ssl::context::no_tlsv1_1);

        // Disable certificate verification for testing
        ctx->set_verify_mode(boost::asio::ssl::verify_none);
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] TLS initialization failed: " << e.what() << std::endl;
    }

    return ctx;
};




// Message handler
auto on_message = [](websocketpp::connection_hdl, client::message_ptr msg) {
    std::cout << "[INFO] Message received: " << msg->get_payload() << std::endl;
};

// Open handler to signal connection success
void on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(mtx);
    connected = true;
    cv.notify_one();
}

TEST(WebSocketClientTest, PollConnectionState) {
    client c;
    c.init_asio();

    // Register TLS handler before connecting
    c.set_tls_init_handler(on_tls_init);

    std::string uri = "wss://echo.websocket.events/";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    ASSERT_FALSE(ec) << "Failed to get connection: " << ec.message();

    c.set_open_handler([](websocketpp::connection_hdl) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "[INFO] Connection opened!" << std::endl;
    connected = true;
    cv.notify_one();  // Notify the condition variable
});


    c.connect(con);

    // Poll connection state (non-blocking alternative)
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (con->get_state() == websocketpp::session::state::open) {
            SUCCEED() << "Connection established.";
            return;
        }
    }

    FAIL() << "Connection did not establish within 10 seconds.";
}


TEST(WebSocketClientTest, ConnectionTest) {
    client c;
    c.init_asio();

    // Register TLS handler immediately after initializing the client
    c.set_tls_init_handler(on_tls_init);

    std::string uri = "wss://echo.websocket.events/";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);

    ASSERT_FALSE(ec) << "Failed to get connection: " << ec.message();

    c.set_open_handler([](websocketpp::connection_hdl) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "[INFO] Connection opened!" << std::endl;
    connected = true;
    cv.notify_one();  // Notify the condition variable
});


    c.set_message_handler(on_message);

    // Connect and run the client event loop directly
    c.connect(con);
    std::cout << "[DEBUG] Running the WebSocket client..." << std::endl;
    EXPECT_NO_THROW(c.run());
}


TEST(WebSocketClientTest, SendTextMessageTest) {
    client c;
    c.init_asio();

    // Register TLS handler before connecting
    c.set_tls_init_handler(on_tls_init);

    std::string uri = "wss://echo.websocket.events/";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    ASSERT_FALSE(ec) << "Failed to get connection: " << ec.message();

    EXPECT_NO_THROW(c.connect(con));

    // Wait for connection with timeout
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(10), [] { return connected; })) {
        std::cerr << "[ERROR] Connection timeout!" << std::endl;
        FAIL() << "WebSocket connection timed out.";
    }

    EXPECT_NO_THROW({
        websocketpp::lib::error_code send_ec;
        c.send(con->get_handle(), "Hello from test!", websocketpp::frame::opcode::text, send_ec);
        ASSERT_FALSE(send_ec) << "Failed to send message: " << send_ec.message();
    });
}
