#include "gronsfeldcipher.h"
#include <string>
#include <vector>

using namespace std;

vector<unsigned char> CipherGronsfeld::encrypt(const vector<unsigned char>& data, const string& key) {
    vector<unsigned char> result;
    
    if (key.empty()) {
        return data;
    }

    for (size_t i = 0; i < data.size(); i++) {
        unsigned char letter = data[i];
        char keyChar = key[i % key.length()];
        int shift = keyChar - '0';
        
        unsigned char newLetter = (letter + shift) % 256;
        result.push_back(newLetter);
    }

    return result;
}

vector<unsigned char> CipherGronsfeld::decrypt(const vector<unsigned char>& data, const string& key) {
    vector<unsigned char> result;

    if (key.empty()) {
        return data;
    }

    for (size_t i = 0; i < data.size(); i++) {
        unsigned char letter = data[i];
        char keyChar = key[i % key.length()];
        int shift = keyChar - '0';
        
        unsigned char oldLetter = (letter - shift + 256) % 256;
        result.push_back(oldLetter);
    }

    return result;
}

PLUGIN_EXPORT CipherAPI* createCipher() {
    return new CipherGronsfeld();
}

PLUGIN_EXPORT void releaseCipher(CipherAPI* cipher) {
    delete cipher;
}
 