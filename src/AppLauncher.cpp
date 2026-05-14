#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "core/DBmanager.h"
#include "core/Encryptor.h"
#include "network/Chat_Server.h"
#include "network/Chat_Client.h"
#include "ui/UI_Engine.h"

std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M");
    return ss.str();
}
int main() {


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
                // ЗАХИСТ ВІД ДУБЛЮВАННЯ:
                // raw_msg має формат "username: текст"
                // Якщо він починається з нашого ніка — просто ігноруємо, бо ми його вже намалювали
                std::string my_prefix = ui.get_username() + ": ";
                if (raw_msg.find(my_prefix) == 0) {
                    return; 
                }

                Message incoming_msg;
                incoming_msg.text = raw_msg;
                incoming_msg.timestamp = get_current_time();
                local_history.push_back(incoming_msg);
                ui.update_messages(local_history); 
            });
        });
        listener_thread.detach(); 



        auto send_logic = [&client, &ui, &local_history](std::string text_to_send) {
            
            auto push_sys_msg = [&](const std::string& sys_text) {
                Message m;
                m.text = sys_text;
                m.timestamp = get_current_time();
                local_history.push_back(m);
            };

            // ПАРСЕР КОМАНД
            if (!text_to_send.empty() && text_to_send[0] == '/') {
                
                if (text_to_send.find("/help") == 0 || text_to_send == "/") {
                    push_sys_msg("System: --- Доступні команди ---");
                    push_sys_msg("System: /change_name [нік] - змінити нікнейм");
                    push_sys_msg("System: /clear - очистити історію чату");
                    push_sys_msg("System: /help - показати цей список");
                } 
                else if (text_to_send.find("/change_name ") == 0) {
                    std::string new_name = text_to_send.substr(13);
                    while (!new_name.empty() && std::isspace(new_name.back())) {
                        new_name.pop_back();
                    }
                    if (!new_name.empty()) {
                        ui.username = new_name;
                        push_sys_msg("System: Нікнейм успішно змінено на " + new_name);
                    } else {
                        push_sys_msg("System: Помилка. Вкажіть нікнейм (наприклад: /change_name lomon)");
                    }
                }
                else if (text_to_send.find("/clear") == 0) {
                    local_history.clear();
                }
                else if (text_to_send.find("/load") == 0) {
                    DBManager db;
                    std::vector<Message> db_history = db.read_all_messages();
                    
                    if (!db_history.empty()) {
                        local_history.clear(); // Очищаємо екран перед завантаженням
                        
                        for (const auto& m : db_history) {
                            Message display_msg;
                            // ДЕШИФРУЄМО збережений текст
                            display_msg.text = Encryptor::decrypt(m.encrypted_content, "my_secure_key_123");
                            // БЕРЕМО ОРИГІНАЛЬНИЙ ЧАС З БАЗИ, а не поточний!
                            display_msg.timestamp = m.timestamp; 
                            
                            local_history.push_back(display_msg);
                        }
                        push_sys_msg("System: БД history.json завантажено (" + std::to_string(db_history.size()) + " пов.).");
                    } else {
                        push_sys_msg("System: Базу не знайдено або вона порожня.");
                    }
                }
                else {
                    // ЛОГУВАННЯ СИРОГО ВВОДУ В ЛАПКАХ (для дебагу)
                    push_sys_msg("System: Невідома команда: '" + text_to_send + "'");
                }

                ui.update_messages(local_history);
                return; 
            }

            // СТАНДАРТНА ВІДПРАВКА
            Message msg;
            msg.text = ui.username + ": " + text_to_send;
            msg.timestamp = get_current_time(); 
            
            client.send_payload(msg);
            
            local_history.push_back(msg);
            ui.update_messages(local_history);
        };
        ui.run_chat_interface(send_logic);
    }

    return 0;
}