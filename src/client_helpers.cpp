#include "client_helpers.h"

std::mutex mtx;
std::condition_variable cv;
bool connected = false;

void on_open(websocketpp::connection_hdl hdl) {
    std::cout << "[DEBUG] Inside on_open function!" << std::endl;
    std::cout << "[INFO] WebSocket connection opened!" << std::endl;

    {
        std::lock_guard<std::mutex> lock(mtx);
        connected = true;
    }
    cv.notify_all();
}


void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        std::cout << "[SERVER] Text: " << msg->get_payload() << std::endl;
    }
}

void on_fail(websocketpp::connection_hdl) {
    std::cout << "[ERROR] WebSocket connection failed!" << std::endl;
}

void on_close(websocketpp::connection_hdl) {
    std::cout << "[INFO] WebSocket connection closed!" << std::endl;
}

context_ptr on_tls_init(websocketpp::connection_hdl) {
    std::cout << "[DEBUG] Initializing TLS context..." << std::endl;

    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cerr << "[ERROR] TLS Initialization Error: " << e.what() << std::endl;
    }

    std::cout << "[DEBUG] TLS context initialized." << std::endl;
    return ctx;
}
