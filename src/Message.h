#ifndef MESSAGE_H
#define MESSAGE_H
#include <string>

class Message {
public:
    int user_id = 0; 
    std::string text;
    std::string timestamp;
    std::string encrypted_content;
};

#endif