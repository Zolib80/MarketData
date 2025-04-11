#include "queue.hpp"
#include <../libs/ixwebsocket/IXWebSocket.h>
#include <string>
#include <mutex>
#include <condition_variable>

class web_socket {
public:
    web_socket(const std::string& url);
    ~web_socket();

    bool connect();
    void close();
    void send(const std::string& message);
    std::string recv();
    
    void setPingOptions();

    int maxReconnectAttempts = 10;
    inline static const std::string URL = "wss://stream-testnet.bybit.com/v5/public/spot";
    inline static const std::string SUBSCRIBE_MESSAGE = R"({"req_id": "test","op": "subscribe","args": ["orderbook.1.BTCUSDT"]})";
    inline static const std::string PING_MESSAGE = R"({"op": "ping"})";

private:
    ix::WebSocket ws;
    std::string receivedMessage;
    std::string pingMessage;
    std::mutex receiveMutex;
    std::mutex configMutex;
    std::condition_variable receiveConditionVariable;
    bool isMessageReceived = false;
    bool isConnected = false;
    int pingIntervalSecs = 20;
};