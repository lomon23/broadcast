#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <functional>
#include <string>
#include <vector>

#include "../Message.h"

class ChatClient {
private:
    std::string server_ip;
    int port;
    int socket_fd;
public: 
    ChatClient(const std::string& ip, int p);
    ~ChatClient();
    // розібрати як працюють конструктори, просто , та в самих хедерах 
    void connect_to_server();
    void send_payload(const Message& msg);
    void receive_loop(std::function<void(const std::string&)> on_message_received);

};
#endif // CHAT_CLIENT_H