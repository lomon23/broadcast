#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <string>
#include <vector>

#include "../Message.h"

class Chat_Client {
private:
    std::string server_ip;
    int port;
    int socket_fg;
public: 
    Chat_Client(const std::string& ip, int p);
    ~Chat_client();
    // розібрати як працюють конструктори, просто , та в самих хедерах 
    void connect();
    void send_payload(const Message& msg);
    void receive_loop();

};
#endif // CHAT_CLIENT_H