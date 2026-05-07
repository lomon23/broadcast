#include "Encryptor.h"
#include <sstream>
#include <iomanip>
#include <cstdlib> // для strtol

std::string Encryptor::encrypt(const std::string& plain_text, const std::string& key) {
    if (key.empty()) return plain_text;

    // 1. Спочатку робимо XOR
    std::string xored = plain_text;
    for (size_t i = 0; i < plain_text.size(); ++i) {
        xored[i] = plain_text[i] ^ key[i % key.size()];
    }

    // 2. Перетворюємо "кашу" з байтів у читабельний Hex-рядок
    std::stringstream ss;
    for (unsigned char c : xored) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

std::string Encryptor::decrypt(const std::string& cipher_text, const std::string& key) {
    if (key.empty() || cipher_text.empty()) return cipher_text;

    // 1. Перетворюємо Hex-рядок назад у байти
    std::string xored;
    for (size_t i = 0; i < cipher_text.length(); i += 2) {
        std::string byteString = cipher_text.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), nullptr, 16);
        xored += byte;
    }

    // 2. Розшифровуємо через XOR
    std::string plain_text = xored;
    for (size_t i = 0; i < xored.size(); ++i) {
        plain_text[i] = xored[i] ^ key[i % key.size()];
    }
    
    return plain_text;
}