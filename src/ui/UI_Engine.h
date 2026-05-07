#ifndef UI_ENGINE_H
#define UI_ENGINE_H
#include <vector>
#include <string>
#include "../Message.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

class UI_Engine {
private:
    std::string current_input;
    std::string username;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::TerminalOutput();
public:
    UI_Engine();

    void run_login_screen();
    void run_chat_interface();
    void update_message(const std::vector<Message>& history);
};

#endif // UI_ENGINE_H