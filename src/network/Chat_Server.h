#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H
#include <string>
#include <vector>
#include "../Message.h"

class ChatServer{
private:
    int port;
    std::vector<int> client_sockets;
    int server_fd;
public:
    ChatServer(int p);
    ~ChatServer();
    
    void listen_loop();
    void broadcast(const Message& msg);
};

#endif // CHAT_CLIENT_H