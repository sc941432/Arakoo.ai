#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

std::mutex mtx;
std::condition_variable cv;
bool connected = false;

void on_open(websocketpp::connection_hdl hdl, client* c) {
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
    } else if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
        std::vector<uint8_t> binary_data(msg->get_payload().begin(), msg->get_payload().end());
        std::cout << "[SERVER] Binary data received of size: " << binary_data.size() << std::endl;
    }
}

void on_fail(websocketpp::connection_hdl hdl) {
    std::cout << "[ERROR] WebSocket connection failed!" << std::endl;
}

void on_close(websocketpp::connection_hdl hdl) {
    std::cout << "[INFO] WebSocket connection closed!" << std::endl;
}

context_ptr on_tls_init(websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << "[ERROR] TLS Initialization Error: " << e.what() << std::endl;
    }
    return ctx;
}

void user_input_thread(client* c, websocketpp::connection_hdl hdl) {
    std::string message;
    while (true) {
        std::cout << "Enter message (or type 'exit' to quit): ";
        std::getline(std::cin, message);
        if (message == "exit") break;
        websocketpp::lib::error_code ec;
        if (message == "binary") {
            std::vector<uint8_t> binary_data = {0x01, 0x02, 0x03, 0x04};
            c->send(hdl, binary_data.data(), binary_data.size(), websocketpp::frame::opcode::binary, ec);
        } else {
            c->send(hdl, message, websocketpp::frame::opcode::text, ec);
        }
        if (ec) {
            std::cout << "[ERROR] Send failed: " << ec.message() << std::endl;
        }
    }
    c->close(hdl, websocketpp::close::status::normal, "User exited");
}

int main() {
    client c;
    std::string uri = "wss://echo.websocket.events/";
    
    try {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.init_asio();
        
        c.set_message_handler(&on_message);
        c.set_tls_init_handler(&on_tls_init);
        c.set_open_handler(bind(&on_open, ::_1, &c));
        c.set_fail_handler(&on_fail);
        c.set_close_handler(&on_close);

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "[ERROR] Connection failed: " << ec.message() << std::endl;
            return 1;
        }
        c.connect(con);

        std::thread ws_thread([&c](){ c.run(); });
        
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return connected; });
        user_input_thread(&c, con->get_handle());
        
        ws_thread.join();
    } catch (websocketpp::exception const & e) {
        std::cout << "[ERROR] WebSocket Exception: " << e.what() << std::endl;
    }
}
