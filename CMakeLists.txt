cmake_minimum_required(VERSION 3.15)
project(BroadcastChat CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v5.0.0
)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(ftxui json)


include_directories(src)

set(SOURCES
    src/AppLauncher.cpp
    src/network/Chat_Client.cpp
    src/network/Chat_Server.cpp
    src/ui/UI_Engine.cpp
    src/core/DBmanager.cpp
    src/core/Encryptor.cpp
)

add_executable(broadcast ${SOURCES})

target_link_libraries(broadcast PRIVATE ftxui::screen ftxui::dom ftxui::component nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(broadcast PRIVATE ws2_32)
endif()
