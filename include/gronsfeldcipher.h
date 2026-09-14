#pragma once
#include "cipherapi.h"

using namespace std;

//класс шифра Гронсфельда, наследуемый от нашего  интерфейса
class CipherGronsfeld : public CipherAPI {
public:
    vector<unsigned char> encrypt(const vector<unsigned char>& data, const string& key) override;
    vector<unsigned char> decrypt(const vector<unsigned char>& data, const string& key) override;
};

extern "C" __declspec(dllexport)
CipherAPI* createCipher();

extern "C" __declspec(dllexport)
void releaseCipher(CipherAPI* cipher);