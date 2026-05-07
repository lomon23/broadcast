#include "UI_Engine.h"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

UI_Engine::UI_Engine() {}

// ======================= ЕКРАН ЛОГІНУ =======================
void UI_Engine::run_login_screen() {
    // 1. Створюємо компонент вводу (прив'язуємо до змінної username)
    Component input_username = Input(&username, "Введіть нікнейм...");

    // 2. Створюємо кнопку. Лямбда-функція виконається при натисканні.
    auto on_click = [this]() {
        if (!username.empty()) {
            screen.Exit(); // Зупиняє Loop, що дозволить програмі піти далі
        }
    };
    Component login_button = Button("Увійти", on_click);

    // 3. Збираємо їх у вертикальний контейнер (щоб клавіатура переключалась між ними)
    Component layout = Container::Vertical({
        input_username,
        login_button,
    });

    // 4. Огортаємо в Рендерер для красивого відображення
    Component component = Renderer(layout, [&] {
        return vbox({
            text("=== BROADCAST CHAT ===") | bold | center,
            separator(),
            hbox(text(" Нікнейм : "), input_username->Render()),
            separator(),
            login_button->Render() | center,
        }) | border | center; // Рамка і центрування по центру терміналу
    });

    // Запускаємо цикл. Програма заблокується тут, поки не викличеться screen.Exit()
    screen.Loop(component);
}

// ======================= ЕКРАН ЧАТУ =======================
void UI_Engine::run_chat_interface(std::function<void(std::string)> on_send_callback) {
    
    // Налаштовуємо інпут так, щоб по натисканню Enter він відправляв текст
    InputOption input_option;
    input_option.on_enter = [this, on_send_callback]() {
        if (!current_input.empty()) {
            on_send_callback(current_input); // Смикаємо мережевий шар
            current_input.clear();           // Очищаємо поле
        }
    };
    
    Component input_field = Input(&current_input, "Напишіть повідомлення...", input_option);

    Component layout = Container::Vertical({
        input_field
    });

    Component component = Renderer(layout, [&] {
        // Формуємо масив візуальних блоків для історії
        Elements msg_elements;
        for (const auto& msg : chat_history) {
            // Формат: [12:00] Нікнейм: Текст
            std::string display_text = "[" + msg.timestamp + "] " + msg.text; 
            msg_elements.push_back(text(display_text));
        }

        return vbox({
            text("Чат (Ви зайшли як: " + username + ")") | bold | center,
            separator(),
            // Блок історії повідомлень. flex - розтягує на весь вільний простір
            // yframe - додає можливість скролити, якщо повідомлень забагато
            vbox(msg_elements) | flex | yframe, 
            separator(),
            hbox(text(" > "), input_field->Render())
        }) | border;
    });

    screen.Loop(component);
}

// ======================= ОНОВЛЕННЯ ДАНИХ =======================
void UI_Engine::update_messages(const std::vector<Message>& history) {
    // ЖОРСТКА АРХІТЕКТУРНА ДЕТАЛЬ:
    // Цей метод буде викликатися з іншого потоку (з receive_loop клієнта).
    // Малювати на екран з фонового потоку не можна — буде Segfault.
    // Тому ми кажемо екрану: "Гей, ось тобі задача (замикання), виконай її в своєму головному потоці".
    
    auto task = [this, history]() {
        this->chat_history = history;
    };
    
    screen.Post(task); // Постановка задачі в чергу UI потоку
}