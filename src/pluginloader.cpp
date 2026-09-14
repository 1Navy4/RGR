#include "pluginloader.h"
#include <iostream>

PluginLoader::PluginLoader() {
    dllHandle = nullptr;
}

PluginLoader::~PluginLoader() {
    if (dllHandle)
        FreeLibrary(dllHandle);
}

CipherAPI* PluginLoader::loadCipher(
    const std::string& dllName,
    ReleaseCipherFunc& releaseFunc)
{
    dllHandle = LoadLibraryA(dllName.c_str());

    if (!dllHandle) {
        // ЕСЛИ DLL НЕ ЗАГРУЗИЛАСЬ, ВЫВОДИМ КОД ОШИБКИ WINDOWS
        std::cout << "\n[СИСТЕМА] Ошибка LoadLibrary для " << dllName 
                  << ". Код ошибки Windows: " << GetLastError() << "\n";
        return nullptr;
    }

    CreateCipherFunc createFunc =
        (CreateCipherFunc)GetProcAddress(dllHandle, "createCipher");

    releaseFunc =
        (ReleaseCipherFunc)GetProcAddress(dllHandle, "releaseCipher");

    if (!createFunc || !releaseFunc) {
        std::cout << "\n[СИСТЕМА] Функции экспорта не найдены в " << dllName 
                  << ". Код ошибки Windows: " << GetLastError() << "\n";
        return nullptr;
    }

    return createFunc();
}
