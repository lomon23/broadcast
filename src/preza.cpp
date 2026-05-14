#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

using namespace ftxui;

int run_presentation() {
    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        " [0] Init        ",
        " [1] Мережа      ",
        " [2] Шифрування  ",
        " [3] База Даних  "
    };
    
    auto tab_selection = Menu(&tab_entries, &tab_index);

    auto tab_content = Renderer([&] {
        switch (tab_index) {
            case 0: return vbox({
                text(" SYSTEM BOOT... OK ") | bold | color(Color::Green),
                separator(),
                text(" Broadcast Chat v1.0 (MVP)"),
                text(""),
                text(" Суть: Захищений канал зв'язку, написаний з нуля."),
                text(" Жодних сторонніх API, 'чорних скриньок' чи HTTP-костилів."),
                text(" Тільки C++, POSIX та пряма робота з пам'яттю.")
            });
            case 1: return vbox({
                text(" NETWORKING & CONCURRENCY ") | bold | color(Color::Cyan),
                separator(),
                text(" -> Зв'язок: Сирі TCP-сокети."),
                text(" -> Багатопотоковість: кожен клієнт живе у своєму std::thread."),
                text(" -> Надійність (Thread Bomb Fix): сервер жорстко вбиває потік"),
                text("    при обриві з'єднання, процесор не лягає від мертвих клієнтів.")
            });
            case 2: return vbox({
                text(" CRYPTO ENGINE ") | bold | color(Color::Red),
                separator(),
                text(" -> Алгоритм: Потокове шифрування (XOR + симетричний ключ)."),
                text(" -> Енкодинг: Байтовий масив конвертується в Hex для JSON."),
                text(""),
                text(" [!] ZERO PLAINTEXT POLICY [!]"),
                text(" Поле відкритого тексту повністю вирізане з мережевих пакетів."),
                text(" Перехоплення трафіку (напр. через Wireshark) дасть"),
                text(" зловмиснику лише набір рандомних Hex-символів.")
            });
            case 3: return vbox({
                text(" DATA STORAGE ") | bold | color(Color::Yellow),
                separator(),
                text(" -> Зберігання: Локальна БД у history.json."),
                text(" -> Принцип 'Data at rest': у файл пишеться ТІЛЬКИ шифр."),
                text(" -> Якщо хтось отримає фізичний доступ до файлу БД,"),
                text("    він не зможе прочитати жодного повідомлення."),
                text(" -> Історія дешифрується локально тільки при старті клієнта.")
            });

            default: return text("");
        }
    });

    auto main_container = Container::Horizontal({tab_selection});
    
    auto renderer = Renderer(main_container, [&] {
        return window(text(" BROADCAST TERMINAL ") | bold | center,
            hbox({
                tab_selection->Render() | border | size(WIDTH, EQUAL, 20),
                tab_content->Render() | border | flex
            })
        ) | color(Color::White); 
    });

    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(renderer);

    return 0;
}