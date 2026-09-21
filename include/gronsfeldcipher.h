#pragma once
#include "cipherapi.h"
#include "plugin_export.h"

using namespace std;

class CipherGronsfeld : public CipherAPI {
public:
    vector<unsigned char> encrypt(const vector<unsigned char>& data, const string& key) override;
    vector<unsigned char> decrypt(const vector<unsigned char>& data, const string& key) override;
};

PLUGIN_EXPORT
CipherAPI* createCipher();

PLUGIN_EXPORT
void releaseCipher(CipherAPI* cipher);
