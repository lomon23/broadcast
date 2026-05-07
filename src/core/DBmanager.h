#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <vector>
#include <string>
#include "../Message.h"

class DBManager {
public:
    // Статичні методи, щоб не створювати об'єкт класу кожен раз
    static void write_message(const Message& msg);
    static std::vector<Message> read_all_messages(); 
};

#endif