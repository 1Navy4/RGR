#pragma once

#include <windows.h>
#include <string>
#include "cipherapi.h"

typedef CipherAPI* (*CreateCipherFunc)();
typedef void (*ReleaseCipherFunc)(CipherAPI*);

class PluginLoader {
private:
    HMODULE dllHandle;

public:
    PluginLoader();
    ~PluginLoader();

    CipherAPI* loadCipher(
        const std::string& dllName,
        ReleaseCipherFunc& releaseFunc);
};