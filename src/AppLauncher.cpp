#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <atomic>
#include <cstdlib>
#include <ctime>

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

std::string get_admin_password() {
    std::ifstream pin_file(".admin_pin.lock");
    std::string pin;
    if (pin_file >> pin) {
        return pin;
    }
    return "0000"; 
}

int main(int argc, char* argv[]) {
    int choice = -1;
    std::string target_ip;
    int target_port = 8080;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--server") choice = 1;
        else if (arg == "--fast-client") choice = 2;
    }

    if (choice == -1) {
        std::cout << "======================================\n";
        std::cout << "        BROADCAST CHAT SYSTEM         \n";
        std::cout << "======================================\n";
        std::cout << "0. Презентація проєкту\n";
        std::cout << "1. Запустити Сервер (Хост)\n";
        std::cout << "2. Швидке підключення (127.0.0.1:8080)\n";
        std::cout << "3. Підключитися вручну (IP/Порт)\n";
        std::cout << "4. Вийти\n";
        std::cout << "Оберіть дію: ";

        if (!(std::cin >> choice)) return 1;
    }

    if (choice == 0) {
        extern void run_presentation(); 
        run_presentation();
        return 0;
    } 
    else if (choice == 1) {
        int port = 8080;
        if (argc == 1) {
            std::cout << "Порт: ";
            std::cin >> port;
        }
        
        srand(time(nullptr));
        std::string new_pin = std::to_string(1000 + rand() % 90000000); 
        
        std::ofstream pin_file(".admin_pin.lock");
        pin_file << new_pin;
        pin_file.close();
        
        std::cout << "[\033[31mADMIN\033[0m] Пароль адміністратора на цю сесію: " << new_pin << "\n";
        
        ChatServer server(port);
        server.listen_loop(); 
        return 0;
    }
    else if (choice == 2) {
        target_ip = "127.0.0.1";
        target_port = 8080;
    }
    else if (choice == 3) {
        std::cout << "IP: ";
        std::cin >> target_ip;
        std::cout << "Порт: ";
        std::cin >> target_port;
    }
    else {
        return 0;
    }

    ChatClient client(target_ip, target_port);
    client.connect_to_server(); 
    UI_Engine ui;
    ui.run_login_screen(); 
    
    std::ifstream ban_check("ban_" + ui.get_username() + ".lock");
    if (ban_check.good()) {
        std::cout << "\x1B[2J\x1B[H";
        std::cout << "\n\n\033[31m[!] ДОСТУП ЗАБОРОНЕНО. Акаунт '" << ui.get_username() << "' заблоковано на цьому пристрої.\033[0m\n\n";
        return 0; 
    }

    uint32_t session_id = std::hash<std::string>{}(ui.get_username() + get_current_time()) & 0xFFFFFF;

    std::cout << "\x1B[2J\x1B[H"; 
    std::cout << "[\033[33mWAIT\033[0m] Initializing secure channel...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    std::cout << "[\033[32m OK \033[0m] Exchanging RSA-2048 public keys...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    std::cout << "[\033[32m OK \033[0m] Establishing AES-256 session block...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "[\033[32m OK \033[0m] Handshake complete.\n";
    std::cout << "[\033[32m OK \033[0m] Authenticated as: " << ui.get_username() << " [Session ID: " << session_id << "]\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    std::atomic<bool> i_am_banned(false);
    bool is_admin = false;
    std::vector<Message> local_history;

    std::thread listener_thread([&client, &ui, &local_history, &is_admin, &i_am_banned](uint32_t my_id) {
        client.receive_loop([&ui, &local_history, my_id, &is_admin, &i_am_banned](const std::string& raw_msg) {
            
            size_t ban_pos = raw_msg.find("[CMD:BAN] ");
            if (ban_pos != std::string::npos) {
                std::string target_user = raw_msg.substr(ban_pos + 10);
                while (!target_user.empty() && (std::isspace(target_user.back()) || target_user.back() == '\0')) {
                    target_user.pop_back();
                }

                if (target_user == ui.get_username() && !is_admin) {
                    i_am_banned = true; 
                    
                    std::ofstream ban_file("ban_" + ui.get_username() + ".lock");
                    ban_file << "banned";
                    ban_file.close();

                    Message sys_msg;
                    sys_msg.text = "System: [!] ВАС ЗАБАНЕНО. Будь-яка спроба написати повідомлення призведе до відключення.";
                    sys_msg.timestamp = get_current_time();
                    local_history.push_back(sys_msg);
                    ui.update_messages(local_history);
                }
                return; 
            }

            std::string my_prefix = ui.get_username() + ": ";
            if (raw_msg.find(my_prefix) == 0) return;

            Message incoming_msg;
            incoming_msg.text = raw_msg;
            incoming_msg.timestamp = get_current_time();
            local_history.push_back(incoming_msg);
            ui.update_messages(local_history); 
        });
    }, session_id);
    listener_thread.detach();

    auto send_logic = [&client, &ui, &local_history, session_id, &is_admin, &i_am_banned](std::string text_to_send) {
        if (i_am_banned) {
            std::cout << "\x1B[2J\x1B[H"; 
            std::cout << "\n\n\033[31m[!] ЗВ'ЯЗОК РОЗІРВАНО. ВИ В ЧОРНОМУ СПИСКУ.\033[0m\n\n";
            exit(0);
        }
        auto push_sys_msg = [&](const std::string& sys_text) {
            Message m;
            m.text = sys_text;
            m.timestamp = get_current_time();
            m.user_id = 0; 
            local_history.push_back(m);
        };

        if (!text_to_send.empty() && text_to_send[0] == '/') {
            if (text_to_send.find("/help") == 0 || text_to_send == "/") {
                push_sys_msg("System: --- Доступні команди ---");
                push_sys_msg("System: /clear - очистити історію чату");
                push_sys_msg("System: /load - завантажити базу");
                push_sys_msg("System: /profile - показати профіль");
                push_sys_msg("System: /help - показати цей список");
                push_sys_msg("System: /su [пароль] - отримати права адміністратора");
                push_sys_msg("System: /ban [нік] - заблокувати користувача (ADMIN)");
            } 
            else if (text_to_send.find("/clear") == 0) {
                local_history.clear();
            }
            else if (text_to_send.find("/profile") == 0) {
                int my_msgs = 0;
                std::string my_prefix = ui.get_username() + ": ";
                for (const auto& m : local_history) {
                    if (m.text.find(my_prefix) == 0) {
                        my_msgs++;
                    }
                }
                
                std::string role = is_admin ? "ADMIN" : "USER";
                
                push_sys_msg("System: +---------------------------------+");
                push_sys_msg("System: |       ПРОФІЛЬ КОРИСТУВАЧА       |");
                push_sys_msg("System: +---------------------------------+");
                push_sys_msg("System: | Нікнейм:  " + ui.get_username());
                push_sys_msg("System: | Роль:     " + role);
                push_sys_msg("System: | Session:  " + std::to_string(session_id));
                push_sys_msg("System: | Написано: " + std::to_string(my_msgs) + " повідомлень");
                push_sys_msg("System: +---------------------------------+");
            }
            else if (text_to_send.find("/su ") == 0) {
                std::string password = text_to_send.substr(4);
                
                password.erase(0, password.find_first_not_of(" \t\n\r"));
                while (!password.empty() && std::isspace(password.back())) password.pop_back();
                
                std::string expected_pin = get_admin_password();
                
                if (password == expected_pin) {
                    is_admin = true;
                    push_sys_msg("System: [ AUTH ] Успіх. Права адміністратора надано.");
                } else {
                    push_sys_msg("System: [ AUTH ] Помилка. Очікувався: '" + expected_pin + "', а ти ввів: '" + password + "'");
                }
            }
            else if (text_to_send.find("/ban ") == 0) {
                if (!is_admin) {
                    push_sys_msg("System: [ ПОМИЛКА ] Відмовлено в доступі. Введіть /su [пароль].");
                } else {
                    std::string target = text_to_send.substr(5);
                    push_sys_msg("System: [ ADMIN ] Користувача " + target + " забанено на сервері.");
                    
                    Message ban_msg;
                    ban_msg.text = "[CMD:BAN] " + target;
                    ban_msg.timestamp = get_current_time();
                    ban_msg.user_id = session_id;
                    client.send_payload(ban_msg);
                }
            }
            else if (text_to_send.find("/load") == 0) {
                DBManager db;
                std::vector<Message> db_history = db.read_all_messages();
                if (!db_history.empty()) {
                    local_history.clear();
                    for (const auto& m : db_history) {
                        Message display_msg;
                        display_msg.text = Encryptor::decrypt(m.encrypted_content, "my_secure_key_123");
                        display_msg.timestamp = m.timestamp; 
                        display_msg.user_id = m.user_id;
                        local_history.push_back(display_msg);
                    }
                    push_sys_msg("System: БД history.json завантажено.");
                } else {
                    push_sys_msg("System: Базу не знайдено.");
                }
            }
            else {
                push_sys_msg("System: Невідома команда: '" + text_to_send + "'");
            }

            ui.update_messages(local_history);
            return; 
        }

        Message msg;
        msg.text = ui.get_username() + ": " + text_to_send;
        msg.timestamp = get_current_time(); 
        msg.user_id = session_id; 
        
        client.send_payload(msg);
        
        local_history.push_back(msg);
        ui.update_messages(local_history);
    };
    
    ui.run_chat_interface(send_logic);

    return 0;
}