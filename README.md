# broadcast

# Про що? 
брод каст, це значення чату в якому немає поділу на приватні чати, це один суцільний чат, в якому всі можуть переписуватись, це нас вертає в гіковські часи 2000х, де цбула база для форумів

# Основна ідея:
створити свій socket сервер на чистому с++ з своїм шифруванням
зробити початкове меню, де ти повинен підключитись по посиланню до цього бродкасту і в ньому вже можна переписуватись, і добавити інструкцію накшталт, склонуйте проект, скачайте таку апку, захосьте, і давайте це посилання своїм друзям/всім

# Stack:
* c++ 17
* FTXUIW
* CMake

# live cycle
Потік А (Main/UI): FTXUI малює чат і чекає, поки ти натиснеш Enter. Коли натиснув — робить send() на сервер.

Потік Б (Receiver): Нескінченний цикл while(true) { recv(...) }. Як тільки від сервера прилетів бродкаст, цей потік оновлює вектор повідомлень у пам'яті, і FTXUI автоматично перемальовує екран.

Клієнт (об'єкт) -> JSON string -> Сокет -> Сервер (JSON string) -> Файл -> Сервер (JSON string) -> Сокет -> Усі клієнти (JSON string) -> Клієнт (об'єкт) -> UI
# Class diagram:
```mermaid
classDiagram
    class Message {
        +int user_id
        +string text
        +string timestamp
        +string encrypted_content
    }

    class DBManager {
        -string file_path
        +write_message(Message msg)
        +read_all_history() vector~Message~
    }

    class Encryptor {
        +string encrypt(string plain_text, string key)
        +string decrypt(string cipher_text, string key)
    }

    class ChatServer {
        -int port
        -vector~int~ client_sockets
        +listen_loop()
        +broadcast(Message msg)
    }

    class ChatClient {
        -string server_ip
        -int port
        +connect()
        +send_payload(Message msg)
        +receive_loop()
    }

    class UIEngine {
        +draw_login_screen()
        +draw_chat_interface()
        +update_messages(vector~Message~ history)
    }

    class AppLauncher {
        +main()
        -bool is_server
        +init_winsock_if_needed()
    }

    AppLauncher --> ChatServer : starts as
    AppLauncher --> ChatClient : starts as
    ChatClient --> UIEngine : provides data to
    ChatServer --> DBManager : persists data
    Encryptor ..> Message : secures content

```
# Squinse diagram
```mermaid
sequenceDiagram
    participant C as Client (User)
    participant UI as UIEngine (FTXUI)
    participant E as Encryptor
    participant S as Server (Your Arch)
    participant DB as DBManager (JSON)

    C->>UI: Вводить текст + Enter
    UI->>E: encrypt("Привіт")
    E-->>UI: "x8jK9..." (encrypted)
    UI->>C: send_payload(Message)
    Note over C,S: Передача через TCP Socket
    C->>S: Encrypted Packet
    S->>DB: write_message(Encrypted)
    DB-->>S: OK (Saved to JSON)
    S->>S: Цикл по всім client_sockets
    S->>C: broadcast(Encrypted Packet)
    Note over S,C: Повернення до всіх клієнтів
    C->>E: decrypt("x8jK9...")
    E-->>C: "Привіт"
    C->>UI: update_chat_area()
    UI-->>C: Відображення на екрані
```