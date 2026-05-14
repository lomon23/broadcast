#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#include <vector>
#include <string>
#include <functional> 
#include "../Message.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

class UI_Engine {
private:
    std::string current_input;

    std::vector<Message> chat_history;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::TerminalOutput();
public:
    UI_Engine();
    std::string username;
    void run_login_screen();
    void run_chat_interface(std::function<void(std::string)> on_send_callback);
    void update_messages(const std::vector<Message>& history);
    std::string get_username() const { return username; }
};

#endif // UI_ENGINE_H