#ifndef ENCRYPTOR_H 
#define ENCRYPTOR_H
#include <string>

class Encryptor{
public:
    static std::string encrypt(const std::string& plain_text, const std::string& key);
    static std::string decrypt(const std::string& cipher_text, const std::string& key);
};

#endif // ENCRYPTOR_H