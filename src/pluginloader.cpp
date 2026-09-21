#include "pluginloader.h"
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>

// Каталог, в котором лежит сам исполняемый файл (bin/).
// Нужен, чтобы плагины находились независимо от текущей рабочей директории.
static std::string getExecutableDir()
{
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0)
        return std::string();

    buffer[length] = '\0';
    std::string path(buffer);

    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos)
        return std::string();

    return path.substr(0, slash);
}

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
    // Если в имени нет '/', dlopen ищет библиотеку только в системных
    // каталогах и не смотрит в текущую папку. Поэтому перебираем пути явно.
    std::vector<std::string> candidates;

    if (dllName.find('/') != std::string::npos) {
        candidates.push_back(dllName);
    }
    else {
        std::string exeDir = getExecutableDir();
        if (!exeDir.empty())
            candidates.push_back(exeDir + "/" + dllName);

        candidates.push_back("./" + dllName);
        candidates.push_back("./bin/" + dllName);
        candidates.push_back(dllName);
    }

    std::string lastError;

    for (const std::string& path : candidates) {
        dllHandle = dlopen(path.c_str(), RTLD_NOW);
        if (dllHandle)
            break;

        const char* err = dlerror();
        lastError = err ? err : "неизвестная ошибка";
    }

    if (!dllHandle) {
        std::cout << "\n[СИСТЕМА] Ошибка dlopen для " << dllName
                  << ". " << lastError << "\n";
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
