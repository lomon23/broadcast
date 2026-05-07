#include "DBmanager.h"
#include <fstream>
#include <nlohmann/json.hpp>

void DBManager::write_message(const Message& msg) {
    std::ofstream file("history.json", std::ios::app);
    if (!file.is_open()) return;
    nlohmann::json j;
    j["user_id"] = msg.user_id;
    j["timestamp"] = msg.timestamp;
    j["encrypted_content"] = msg.encrypted_content;
    file << j.dump() << "\n";
}

std::vector<Message> DBManager::read_all_messages() {
    std::ifstream file("history.json");
    std::vector<Message> history;
    if (!file.is_open()) return history;
    std::string line;
    while (std::getline(file, line)) {
        try {
            auto j = nlohmann::json::parse(line);
            Message m;
            m.user_id = j["user_id"];
            m.timestamp = j["timestamp"];
            m.encrypted_content = j["encrypted_content"];
            history.push_back(m);
        } catch (...) {}
    }
    return history;
}