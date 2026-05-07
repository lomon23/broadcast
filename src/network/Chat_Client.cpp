#include "Chat_Client.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include "../core/Encryptor.h"


using json = nlohmann::json;

ChatClient::ChatClient(const std::string& ip, int p) : server_ip(ip), port(p), socket_fd(-1) {}

ChatClient::~ChatClient() { if (socket_fd != -1) close(socket_fd); }

void ChatClient::connect_to_server() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr);
    connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
}

void ChatClient::send_payload(const Message& msg) {
    if (socket_fd == -1) return;
    std::string key = "my_secure_key_123";
    
    json j;
    j["user_id"] = msg.user_id;
    j["timestamp"] = msg.timestamp;
    j["encrypted_content"] = Encryptor::encrypt(msg.text, key);
    std::string payload = j.dump();
    send(socket_fd, payload.c_str(), payload.size(), 0);
}

void ChatClient::receive_loop(std::function<void(const std::string&)> on_msg) {
    char buffer[4096]; 
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        std::string raw_data(buffer, bytes);
        
        std::stringstream ss(raw_data);
        std::string line;
        
        while (std::getline(ss, line, '}')) { 
            if (line.empty() || line.find('{') == std::string::npos) continue;
            line += "}"; 
            try {
                auto j = nlohmann::json::parse(line);
                std::string decrypted = Encryptor::decrypt(j["encrypted_content"], "my_secure_key_123");
                
                std::string display = "[" + j.value("timestamp", "History") + "] " + decrypted;
                on_msg(display);
            } catch (...) {
                if (line.size() > 2) on_msg(line);
            }
        }
    }
}