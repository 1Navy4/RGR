#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cstdio>
#include <numeric>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "pluginloader.h"
#include "keygenerator.h"

#undef max
#undef min

using namespace std;
using namespace SenCipher;

enum class MainMenu{
    Exit = 0,
    Text = 1,
    File = 2,
    KeyGenerator = 3
};

enum class Algorithm{
    Back = 0,
    Gronsfeld = 1,
    RailFence = 2
};

enum class Mode{
    Back = 0,
    Encrypt = 1,
    Decrypt = 2
};

// ---- Русский текст в консоли Windows -----------------------------------
//
// Раньше программа дёргала _setmode(..., _O_U16TEXT) туда-сюда: включала
// широкий режим только на время чтения одной строки, а всё остальное
// время работала через обычные cout/cin в кодовой странице CP_UTF8.
// Из-за этого переключения буфер консоли путался: то пропадали меню,
// то ломался ввод кириллицы.
//
// Решение проще: включаем _O_U16TEXT один раз в начале main() и больше
// не трогаем режим консоли. Все меню, подсказки и ввод/вывод теперь
// идут через wcout/wcin (широкие строки, L"..."). Байты для шифра
// (vector<unsigned char>) по-прежнему получаем и отдаём в UTF-8 —
// конвертация происходит только в двух местах: перед вызовом DLL
// и после получения результата дешифровки.

#ifdef _WIN32
wstring utf8ToWide(const string& utf8){
    if (utf8.empty()) return wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &result[0], size);
    return result;
}

string wideToUtf8(const wstring& wide){
    if (wide.empty()) return string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), &result[0], size, nullptr, nullptr);
    return result;
}
#else
wstring utf8ToWide(const string& utf8){
    return wstring(utf8.begin(), utf8.end());
}
string wideToUtf8(const wstring& wide){
    return string(wide.begin(), wide.end());
}
#endif

void waitForEnter(){
    wcout << L"\n====================================\n";
    wcout << L"Нажмите Enter для продолжения.";
    wcout << L"\n====================================\n";
    wcin.ignore(numeric_limits<streamsize>::max(),L'\n');
    wcin.get();
}

void printSuccess(const wstring& message){
    wcout << L"\n====================================\n";
    wcout << L"[УСПЕХ]\n";
    wcout << message << endl;
    wcout << L"====================================\n";
}

void printError(const wstring& message){
    wcout << L"\n====================================\n";
    wcout << L"[ОШИБКА]\n";
    wcout << message << endl;
    wcout << L"====================================\n";
}

vector<unsigned char> readFile(const string& fileName){
    ifstream file(fileName, ios::binary);
    if (!file){
        throw runtime_error("Не удалось открыть файл.");
    }
    return vector<unsigned char>(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
}

void writeFile(const string& fileName, const vector<unsigned char>& data){
    ofstream file(fileName, ios::binary);
    if (!file){
        throw runtime_error("Не удалось создать файл.");
    }
    file.write(reinterpret_cast<const char*>(data.data()),static_cast<streamsize>(data.size()));
}

vector<unsigned char> stringToBytes(const string& text){
    return vector<unsigned char>(text.begin(), text.end());
}

vector<unsigned char> hexToBytes(const string& hexString){
    vector<unsigned char> result;
    for (size_t i = 0; i < hexString.length(); i += 2){
        string part = hexString.substr(i, 2);
        result.push_back(static_cast<unsigned char>(stoi(part, nullptr, 16)));
    }
    return result;
}

string bytesToString(const vector<unsigned char>& data){
    return string(data.begin(),data.end());
}

// Читает "слово" (ключ, имя файла) без пробелов через широкий поток
// и возвращает обычную string.
string readConsoleWordAscii(){
    wstring wtemp;
    wcin >> wtemp;
    return wideToUtf8(wtemp);
}

// Читает целую строку (может содержать кириллицу) через широкий поток
// и возвращает её в UTF-8.
string readConsoleLineUtf8(){
    wstring wline;
    getline(wcin, wline);
    return wideToUtf8(wline);
}

// Печатает UTF-8 строку (например, результат дешифровки) в консоль.
void printConsoleUtf8(const string& utf8text){
    wcout << utf8ToWide(utf8text) << endl;
}

int showMenu(const wstring& title,
             const vector<wstring>& items,
             bool showBack = true){

    wcout << L"\n====================================\n";
    wcout << title << L'\n';
    wcout << L"====================================\n\n";

    int number = 1;
    for (const auto& item : items){
        wcout << number++ << L". " << item << L'\n';
    }

    if (showBack){
        wcout << L"\n0. Назад\n";
    }

    wcout << L"\nВаш выбор: ";
    int choice;
    wcin >> choice;
    return choice;
}

Algorithm chooseAlgorithm(){
    return static_cast<Algorithm>(
        showMenu(
            L"Выберите алгоритм",
            {
                L"Шифр Гронсфельда",
                L"Шифр Зигзаг"
            }
        )
    );
}

void keyGeneratorMenu(){
    wcout << L"\nГенератор ключей\n\n";
    wcout << L"1. Шифр Гронсфельда\n";
    wcout << L"2. Шифр Зигзаг\n";
    wcout << L"\n0. Назад\n";
    wcout << L"Ваш выбор: ";
    int temp;
    wcin >> temp;
    Algorithm choice = static_cast<Algorithm>(temp);

    switch (choice){
        case Algorithm::Gronsfeld:{
            string key = generateGronsfeldKey(8);
            wcout << L"\nКлюч Гронсфельда: " << utf8ToWide(key) << endl;
            break;
        }
        case Algorithm::RailFence:{
            wcout << L"\nКоличество строк: " << generateRailFenceKey() << endl;
            break;
        }
        case Algorithm::Back:
            return;
        default:
            printError(L"Некорректный выбор.");
            break;
    }
    waitForEnter();
}

vector<unsigned char> processDLL(const string& dllName, const vector<unsigned char>& data, bool encryptMode, const string& key){
    PluginLoader loader;
    ReleaseCipherFunc releaseFunc;
    CipherAPI* cipher = loader.loadCipher(dllName, releaseFunc);
    if (!cipher){
        throw runtime_error("Не удалось загрузить плагин: " + dllName);
    }

    vector<unsigned char> result;
    if (encryptMode) {
        result = cipher->encrypt(data, key);
    }
    else {
        result = cipher->decrypt(data, key);
    }
    releaseFunc(cipher);
    return result;
}

vector<unsigned char> processGronsfeld(const vector<unsigned char>& data, bool encryptMode){
    wcout << L"\nВведите цифровой ключ: ";
    string key = readConsoleWordAscii();
    return processDLL("gronsfeldcipher.dll", data, encryptMode, key);
}

vector<unsigned char> processRailFence(const vector<unsigned char>& data, bool encryptMode){
    wcout << L"\nВведите количество строк: ";
    string key = readConsoleWordAscii();
    return processDLL("railfencecipher.dll", data, encryptMode, key);
}

void fileMenu(){
    Algorithm algorithm = chooseAlgorithm();
    if (algorithm == Algorithm::Back){
        return;
    }
    Mode mode = static_cast<Mode>(showMenu(L"Выберите режим",{
        L"Шифрование",
        L"Дешифрование"
        }));
    if (mode == Mode::Back){
        return;
    }
    wcout << L"\nВведите имя файла:\n";
    string inputFile = readConsoleWordAscii();

    try {
        vector<unsigned char> data = readFile(inputFile);
        vector<unsigned char> result;

        if (algorithm == Algorithm::Gronsfeld){
            result = processGronsfeld(data, mode == Mode::Encrypt);
        }
        else if (algorithm == Algorithm::RailFence){
            result = processRailFence(data, mode == Mode::Encrypt);
        }

        string outputFile;
        if (mode == Mode::Encrypt){
            outputFile = "encrypted_" + inputFile;
        }
        else{
            outputFile = "decrypted_" + inputFile;
        }
        writeFile(outputFile,result);
        printSuccess(L"Файл сохранён как:\n" + utf8ToWide(outputFile));
    }
    catch (const exception& e) {
        printError(utf8ToWide(e.what()));
    }
    waitForEnter();
}

void textMenu(){
    Algorithm algorithm = chooseAlgorithm();
    if (algorithm == Algorithm::Back){
        return;
    }
    Mode mode = static_cast<Mode>(
    showMenu(L"Выберите режим",{
            L"Шифрование",
            L"Дешифрование"
        }));
    if (mode == Mode::Back){
        return;
    }

    wcin.ignore((numeric_limits<streamsize>::max)(), L'\n');

    string text;
    if (mode == Mode::Encrypt){
        wcout << L"\nВведите текст:\n";
        text = readConsoleLineUtf8();
    }
    else{
        wcout << L"\nВведите HEX:\n";
        text = readConsoleLineUtf8();
    }

    if (text.empty()) {
        printError(L"Строка ввода пуста.");
        waitForEnter();
        return;
    }

    vector<unsigned char> data;
    if (mode == Mode::Encrypt){
        data = stringToBytes(text);
    }
    else{
        try {
            data = hexToBytes(text);
        } catch(...) {
            printError(L"Неверный HEX-формат.");
            waitForEnter();
            return;
        }
    }

    try {
        vector<unsigned char> result;

        if (algorithm == Algorithm::Gronsfeld){
            result = processGronsfeld(data, mode == Mode::Encrypt);
        }
        else if (algorithm == Algorithm::RailFence){
            result = processRailFence(data, mode == Mode::Encrypt);
        }

        if (mode == Mode::Encrypt){
            wcout << L"\nРезультат (HEX):\n";
            wchar_t buf[3];
            for (unsigned char byte : result){
                swprintf(buf, 3, L"%02X", byte);
                wcout << buf;
            }
            wcout << endl;
        }
        else{
            wcout << L"\nРезультат:\n";
            printConsoleUtf8(bytesToString(result));
        }
    }
    catch (const exception& e) {
        printError(utf8ToWide(e.what()));
    }
    waitForEnter();
}

int main(){
    #ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    #endif

    while (true){
        int choice = showMenu(
        L"Система шифрования данных Sencipher",{
            L"Работа с текстом",
            L"Работа с файлами",
            L"Генератор ключей"
        }, true);

        MainMenu menuChoice = static_cast<MainMenu>(choice);

        if (choice == 0 || menuChoice == MainMenu::Exit) {
            wcout << L"\nЗавершение программы.\n";
            return 0;
        }

        switch (menuChoice){
            case MainMenu::Text:
                textMenu();
                break;
            case MainMenu::File:
                fileMenu();
                break;
            case MainMenu::KeyGenerator:
                keyGeneratorMenu();
                break;
            default:
                printError(L"Некорректный выбор.");
                waitForEnter();
                break;
        }
    }
    return 0;
}