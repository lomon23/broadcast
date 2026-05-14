#include "UI_Engine.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
using namespace ftxui;

UI_Engine::UI_Engine() : screen(ScreenInteractive::Fullscreen()) {}

void UI_Engine::run_login_screen() {
    Component input_username = Input(&username, "Введіть нікнейм...");

    auto on_click = [this]() {
        if (!username.empty()) {
            screen.Exit(); 
        }
    };
    Component login_button = Button("Увійти", on_click);

    Component layout = Container::Vertical({
        input_username,
        login_button,
    });

    Component component = Renderer(layout, [&] {
        return vbox({
            text("=== BROADCAST CHAT ===") | bold | center,
            separator(),
            hbox(text(" Нікнейм : "), input_username->Render()),
            separator(),
            login_button->Render() | center,
        }) | border | center; 
    });

    screen.Loop(component);
}

void UI_Engine::run_chat_interface(std::function<void(std::string)> on_send_callback) {
    InputOption input_option;
    input_option.on_enter = [this, on_send_callback]() {
        if (!current_input.empty()) {
            on_send_callback(current_input); 
            current_input.clear();           
        }
    };
    
    Component input_field = Input(&current_input, " Напишіть повідомлення...", input_option);

    Component layout = Container::Vertical({
        input_field
    });

    Component component = Renderer(layout, [&] {
        Elements msg_elements;
        
        // --- ДИНАМІЧНИЙ АВТО-СКРОЛ ---
        // 1. Отримуємо висоту вікна терміналу
        int terminal_height = Terminal::Size().dimy;
        // 2. Віднімаємо 9 рядків (рамки, хедер, лінія вводу і сепаратори)
        int max_messages = std::max(1, terminal_height - 9); 
        
        // 3. Знаходимо індекс, з якого починати малювати, щоб влізли тільки найсвіжіші
        int start_idx = 0;
        if ((int)chat_history.size() > max_messages) {
            start_idx = (int)chat_history.size() - max_messages;
        }

        // Ітеруємося тільки по видимій частині історії
        for (int i = start_idx; i < (int)chat_history.size(); ++i) {
            const auto& msg = chat_history[i];
            
            std::string clean_text = msg.text;
            if (clean_text.size() >= 8 && clean_text[0] == '[' && clean_text[3] == ':' && clean_text[6] == ']' && clean_text[7] == ' ') {
                clean_text = clean_text.substr(8);
            }

            std::string sender_name = "System";
            std::string message_content = clean_text;
            size_t colon_pos = clean_text.find(": ");
            
            if (colon_pos != std::string::npos) {
                sender_name = clean_text.substr(0, colon_pos);
                message_content = clean_text.substr(colon_pos + 2);
            }

            auto time_el = text("[" + msg.timestamp + "] ") | color(Color::GrayDark);
            
            Element name_el;
            if (sender_name == this->username) {
                name_el = text(sender_name + ": ") | bold | color(Color::Green);
            } else if (sender_name == "System") {
                name_el = text(sender_name + ": ") | bold | color(Color::Yellow);
            } else {
                name_el = text(sender_name + ": ") | bold | color(Color::White);
            }

            Elements text_parts;
            size_t current_pos = 0;
            
            while (current_pos < message_content.length()) {
                size_t http_pos = message_content.find("http://", current_pos);
                size_t https_pos = message_content.find("https://", current_pos);
                size_t link_pos = std::min(http_pos, https_pos);

                // Якщо лінків більше немає — закидаємо залишок тексту і виходимо
                if (link_pos == std::string::npos) {
                    text_parts.push_back(text(message_content.substr(current_pos)) | color(Color::White));
                    break;
                }

                // Додаємо текст ДО лінка
                if (link_pos > current_pos) {
                    text_parts.push_back(text(message_content.substr(current_pos, link_pos - current_pos)) | color(Color::White));
                }

                // Шукаємо кінець лінка (перший пробіл після нього, або кінець рядка)
                size_t space_pos = message_content.find(' ', link_pos);
                if (space_pos == std::string::npos) space_pos = message_content.length();

                // Вирізаємо сам лінк і робимо його синім та підкресленим
                std::string link_url = message_content.substr(link_pos, space_pos - link_pos);
                text_parts.push_back(text(link_url) | color(Color::Blue) | underlined);

                current_pos = space_pos; // Зсуваємо курсор парсера
            }

            // Збираємо розпарсений текст в один горизонтальний блок
            auto text_el = hbox(text_parts);
            msg_elements.push_back(hbox({time_el, name_el, text_el}));
        }

        return vbox({
            // Хедер
            hbox({
                text(" BROADCAST CHAT ") | bold,
                filler(), 
                text("Користувач: ") | color(Color::GrayDark),
                text(username) | bold | color(Color::Green),
                text(" ")
            }),
            separator(),
            
            // ЧАТ: Повідомлення зверху, філлер знизу «з'їдає» пустоту і притискає інпут
            vbox({
                vbox(msg_elements),
                filler() 
            }) | yframe, 
            
            
            // ІНПУТ
            hbox({
                text(" > ") | bold | color(Color::Green),
                input_field->Render()
            })
        }) | border;
    });
    screen.Loop(component);
}

void UI_Engine::update_messages(const std::vector<Message>& history) {
    auto task = [this, history]() {
        this->chat_history = history;
    };
    
    screen.Post(task); 
}