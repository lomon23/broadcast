#ifndef ENCRYPTOR_H 
#define ENCRYPTOR_H
#inclde <string>

class Encryptor{
public:
    static std::string encrypt(const std::strin& plain_text, const std::string& key);
    static std::string decrypt(const std::string& cipher_text, const std::string& key);
};

#endif // ENCRYPTOR_H