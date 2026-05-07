#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <string>
#include <vector>
#include "../Message.h"
class DBManager {
private:
    static inline const std::string file_path = "history.json";
    // розібрати чому static inline const 
public:
    static void write_message(const Message& msg);
    static std::vector<Message> read_all_history();
    // розібрати як в зхедерах робити вектори  
};

#endif // DBMANAGER_H

