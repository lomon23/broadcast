#include "UI_Engine.h"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

UI_Engine::UI_Engine() {}

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
    
    Component input_field = Input(&current_input, "Напишіть повідомлення...", input_option);

    Component layout = Container::Vertical({
        input_field
    });

    Component component = Renderer(layout, [&] {
        Elements msg_elements;
        for (const auto& msg : chat_history) {
            std::string display_text = "[" + msg.timestamp + "] " + msg.text; 
            msg_elements.push_back(text(display_text));
        }

        return vbox({
            text("Чат (Ви зайшли як: " + username + ")") | bold | center,
            separator(),
            vbox(msg_elements) | flex | yframe, 
            separator(),
            hbox(text(" > "), input_field->Render())
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