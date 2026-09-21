#pragma once

#include <string>
#include "cipherapi.h"

typedef CipherAPI* (*CreateCipherFunc)();
typedef void (*ReleaseCipherFunc)(CipherAPI*);

class PluginLoader {
private:
    void* dllHandle;

public:
    PluginLoader();
    ~PluginLoader();

    CipherAPI* loadCipher(
        const std::string& dllName,
        ReleaseCipherFunc& releaseFunc);
};
