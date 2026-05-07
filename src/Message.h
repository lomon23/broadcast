#ifndef MESSAGE_H
#define MESSAGE_H
#include <string>

class Message{
public:
    int user_id;
    std::string text;
    std::string timestamp;
    std::string encrypted_content;
};

#endif // MESSAGE_H