#include "queue.h"
#include <../libs/ixwebsocket/IXWebSocket.h>
#include <string>
#include <mutex>

class WebSocket {
public:
    WebSocket(const std::string& url);
    ~WebSocket();

    bool connect();
    void close();
    void send(const std::string& message);
    bool recv(std::vector<std::string>& messages);
    
    void set_ping_options();

    int const MAX_RECONNECT_ATTEMPTS = 10;
    inline static const std::string URL = "wss://stream-testnet.bybit.com/v5/public/spot";
    inline static const std::string SUBSCRIBE_MESSAGE = R"({"req_id": "test","op": "subscribe","args": ["orderbook.50.BTCUSDT"]})";
    inline static const std::string PING_MESSAGE = R"({"op": "ping"})";

private:
    ix::WebSocket ws_;
    std::vector<std::string> received_messages_;
    std::string ping_message_;
    std::mutex receive_mutex_;
    bool is_connected_ = false;
    int ping_interval_secs_ = 20;
};