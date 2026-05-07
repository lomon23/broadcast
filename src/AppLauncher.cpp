#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include "network/Chat_Server.h"
#include "network/Chat_Client.h"
#include "ui/UI_Engine.h"


int main() {
    init_winsock_if_needed();

    std::cout << "======================================\n";
    std::cout << "        BROADCAST CHAT SYSTEM         \n";
    std::cout << "======================================\n";
    std::cout << "0. Презентація проєкту\n";
    std::cout << "1. Запустити Сервер (Хост)\n";
    std::cout << "2. Підключитися до Чату (Клієнт)\n";
    std::cout << "Оберіть дію: ";

    int choice;
    if (!(std::cin >> choice)) return 1;

    if (choice == 0) {
        extern void run_presentation(); 
        run_presentation();
    } 
    else if (choice == 1) {
        int port;
        std::cout << "Порт: ";
        std::cin >> port;
        ChatServer server(port);
        server.listen_loop(); 
    } 
    else if (choice == 2) {
        std::string ip;
        int port;
        std::cout << "IP: ";
        std::cin >> ip;
        std::cout << "Порт: ";
        std::cin >> port;

        ChatClient client(ip, port);
        client.connect_to_server(); 

        UI_Engine ui;
        ui.run_login_screen(); 

        std::vector<Message> local_history;

        std::thread listener_thread([&client, &ui, &local_history]() {
            client.receive_loop([&ui, &local_history](const std::string& raw_msg) {
                Message incoming_msg;
                incoming_msg.text = raw_msg;
                incoming_msg.timestamp = "Вхідне";
                local_history.push_back(incoming_msg);
                ui.update_messages(local_history); 
            });
        });
        listener_thread.detach(); 

        auto send_logic = [&client, &ui, &local_history](std::string text_to_send) {
            Message msg;
            msg.text = ui.get_username() + ": " + text_to_send;
            msg.timestamp = "Я"; 
            client.send_payload(msg);
            local_history.push_back(msg);
            ui.update_messages(local_history);
        };

        ui.run_chat_interface(send_logic);
    }

    cleanup_winsock();
    return 0;
}