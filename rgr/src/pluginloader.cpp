#include "pluginloader.h"
#include <iostream>
#include <dlfcn.h>

PluginLoader::PluginLoader() {
    dllHandle = nullptr;
}

PluginLoader::~PluginLoader() {
    if (dllHandle)
        dlclose(dllHandle);
}

CipherAPI* PluginLoader::loadCipher(
    const std::string& dllName,
    ReleaseCipherFunc& releaseFunc)
{
   
    dllHandle = dlopen(dllName.c_str(), RTLD_NOW);

    if (!dllHandle) {
        std::cout << "\n[СИСТЕМА] Ошибка dlopen для " << dllName
                  << ". " << dlerror() << "\n";
        return nullptr;
    }

    dlerror();

    CreateCipherFunc createFunc =
        (CreateCipherFunc)dlsym(dllHandle, "createCipher");

    releaseFunc =
        (ReleaseCipherFunc)dlsym(dllHandle, "releaseCipher");

    const char* symError = dlerror();
    if (!createFunc || !releaseFunc || symError) {
        std::cout << "\n[СИСТЕМА] Функции экспорта не найдены в " << dllName
                  << (symError ? (std::string(". ") + symError) : "") << "\n";
        return nullptr;
    }

    return createFunc();
}
