#include "Chat_Server.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <algorithm>
#include "../core/DBmanager.h"
#include <nlohmann/json.hpp>

ChatServer::ChatServer(int p) : port(p), server_fd(-1) {}

ChatServer::~ChatServer() {
    for (int client_fd : client_sockets) close(client_fd);
    if (server_fd != -1) close(server_fd);
}

void ChatServer::listen_loop() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) return;
    listen(server_fd, 10);
    std::cout << "[Server] Listening on port " << port << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int new_client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (new_client_fd < 0) continue;

        client_sockets.push_back(new_client_fd);
        std::cout << "[Server] Client connected: " << new_client_fd << "\n";

        // Відправляємо історію
        std::vector<Message> history = DBManager::read_all_messages();
        for (const auto& m : history) {
            nlohmann::json j;
            j["user_id"] = m.user_id;
            j["timestamp"] = m.timestamp;
            j["encrypted_content"] = m.encrypted_content;
            std::string payload = j.dump();
            send(new_client_fd, payload.c_str(), payload.size(), 0);
            usleep(5000); 
        }

        std::thread([this, new_client_fd]() {
            char buffer[2048];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(new_client_fd, buffer, sizeof(buffer), 0);
                
                if (bytes <= 0) { // ФІКС: Клієнт відпав
                    close(new_client_fd);
                    client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), new_client_fd), client_sockets.end());
                    break; 
                }

                try {
                    auto j = nlohmann::json::parse(std::string(buffer, bytes));
                    Message msg;
                    msg.user_id = j.value("user_id", 0);
                    msg.timestamp = j.value("timestamp", "00:00");
                    msg.encrypted_content = j["encrypted_content"];

                    DBManager::write_message(msg);

                    for (int target : client_sockets) {
                        if (target != new_client_fd) send(target, buffer, bytes, 0);
                    }
                } catch (...) {}
            }
        }).detach();
    }
}