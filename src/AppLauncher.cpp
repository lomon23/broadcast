#include <iostream>
#include <string>

// Підключаємо твої класи (шляхи відповідно до структури src/)
#include "network/Chat_Server.h"
#include "network/Chat_Client.h"
#include "ui/UI_Engine.h"

// ======================================================================
// МАГІЯ ДЛЯ WINDOWS (щоб .exe не впав на компі викладача)
// ======================================================================
#ifdef _WIN32
    #include <winsock2.h>
    // Ця прагма говорить компілятору Windows підтягнути бібліотеку ws2_32
    #pragma comment(lib, "ws2_32.lib") 
#endif

void init_winsock_if_needed() {
#ifdef _WIN32
    WSADATA wsaData;
    // Запитуємо версію сокетів 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Error] WSAStartup failed. Network initialization error.\n";
        exit(1);
    }
#endif
}

void cleanup_winsock() {
#ifdef _WIN32
    WSACleanup();
#endif
}
// ======================================================================

int main() {
    // 1. Обов'язкова ініціалізація для Windows (на Linux це просто пуста функція)
    init_winsock_if_needed();

    std::cout << "======================================\n";
    std::cout << "        BROADCAST CHAT SYSTEM         \n";
    std::cout << "======================================\n";
    std::cout << "1. Запустити Сервер (Хост)\n";
    std::cout << "2. Підключитися до Чату (Клієнт)\n";
    std::cout << "Оберіть дію (1 або 2): ";

    int choice;
    if (!(std::cin >> choice)) {
        std::cerr << "Некоректний ввід.\n";
        return 1;
    }

    if (choice == 1) {
        // ================= РЕЖИМ СЕРВЕРА =================
        int port;
        std::cout << "Введіть порт для прослуховування (наприклад, 8080): ";
        std::cin >> port;

        std::cout << "\n[System] Starting Server on port " << port << "...\n";
        
        ChatServer server(port);
        // Запускаємо нескінченний цикл сервера
        server.listen_loop(); 

    } else if (choice == 2) {
        // ================= РЕЖИМ КЛІЄНТА =================
        std::string ip;
        int port;
        
        std::cout << "Введіть IP сервера (127.0.0.1 для локального тесту): ";
        std::cin >> ip;
        std::cout << "Введіть порт: ";
        std::cin >> port;

        ChatClient client(ip, port);
        client.connect_to_server(); 

        UI_Engine ui;
        ui.run_login_screen(); 

        // Створюємо локальний масив історії чату для відмальовки
        std::vector<Message> local_history;

        // ---------------------------------------------------------
        // СТВОРЮЄМО ФОНОВИЙ ПОТІК ДЛЯ ПРОСЛУХОВУВАННЯ МЕРЕЖІ
        // ---------------------------------------------------------
        std::thread listener_thread([&client, &ui, &local_history]() {
            // Цей код буде крутитися паралельно інтерфейсу
            client.receive_loop([&ui, &local_history](const std::string& raw_msg) {
                // Коли приходить пакет, пакуємо його в Message 
                // (тимчасово, поки ти не прикрутив повноцінний JSON парсинг)
                Message incoming_msg;
                incoming_msg.text = raw_msg;
                incoming_msg.timestamp = "Вхідне";
                
                local_history.push_back(incoming_msg);
                
                // Передаємо історії в UI. FTXUI безпечно оновить екран
                ui.update_messages(local_history); 
            });
        });
        
        // Відв'язуємо потік від головного, щоб він жив своїм життям
        listener_thread.detach(); 

        // ---------------------------------------------------------
        // ЛОГІКА ВІДПРАВКИ (Коли тиснеш Enter)
        // ---------------------------------------------------------
        auto send_logic = [&client, &ui, &local_history](std::string text_to_send) {
            Message msg;
            msg.text = ui.get_username() + ": " + text_to_send;
            msg.timestamp = "Я"; 
            
            client.send_payload(msg);

            // Оскільки твій сервер розсилає повідомлення всім КРІМ відправника,
            // ми маємо самостійно додати своє ж повідомлення на власний екран
            local_history.push_back(msg);
            ui.update_messages(local_history);
        };

        // Запускаємо головний нескінченний цикл UI (блокує потік тут)
        ui.run_chat_interface(send_logic);

    } else {
        std::cout << "Невідомий вибір. Завершення програми.\n";
    }

    // 2. Коректно закриваємо ресурси ОС перед виходом
    cleanup_winsock();
    return 0;
}