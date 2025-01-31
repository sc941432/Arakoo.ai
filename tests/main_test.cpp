#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "../src/main.cpp"

// Test for WebSocket client connection
TEST_CASE("WebSocket Client Connection Test") {
    client c;
    std::string uri = "wss://echo.websocket.events/";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);

    REQUIRE_FALSE(ec);  // Check that no error occurred

    c.set_open_handler([](websocketpp::connection_hdl) {
        std::cout << "Connection opened!" << std::endl;
    });

    REQUIRE_NOTHROW(c.connect(con));
}

// Test for sending a text message
TEST_CASE("WebSocket Client Send Text Message Test") {
    client c;
    std::string uri = "wss://echo.websocket.events/";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);

    REQUIRE_FALSE(ec);

    c.connect(con);

    REQUIRE_NOTHROW([&]() {
        websocketpp::lib::error_code send_ec;
        c.send(con->get_handle(), "Hello from test!", websocketpp::frame::opcode::text, send_ec);
        REQUIRE_FALSE(send_ec);
    }());

}
