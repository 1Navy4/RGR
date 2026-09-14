#pragma once
#include <vector>
#include <string>

class CipherAPI {
public:
    virtual ~CipherAPI() {}

    virtual std::vector<unsigned char> encrypt(
        const std::vector<unsigned char>& data,
        const std::string& key) = 0;

    virtual std::vector<unsigned char> decrypt(
        const std::vector<unsigned char>& data,
        const std::string& key) = 0;
};