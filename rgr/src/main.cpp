#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cstdio>
#include <numeric>
#include <vector>
#include <clocale>

#include "pluginloader.h"
#include "keygenerator.h"

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

void waitForEnter(){
    cout << "\n====================================\n";
    cout << "Нажмите Enter для продолжения.";
    cout << "\n====================================\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void printSuccess(const string& message){
    cout << "\n====================================\n";
    cout << "[УСПЕХ]\n";
    cout << message << endl;
    cout << "====================================\n";
}

void printError(const string& message){
    cout << "\n====================================\n";
    cout << "[ОШИБКА]\n";
    cout << message << endl;
    cout << "====================================\n";
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
    file.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
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
    return string(data.begin(), data.end());
}

string readConsoleWordAscii(){
    string temp;
    cin >> temp;
    return temp;
}

string readConsoleLineUtf8(){
    string line;
    getline(cin, line);
    return line;
}

void printConsoleUtf8(const string& utf8text){
    cout << utf8text << endl;
}

int showMenu(const string& title,
             const vector<string>& items,
             bool showBack = true){

    cout << "\n====================================\n";
    cout << title << '\n';
    cout << "====================================\n\n";

    int number = 1;
    for (const auto& item : items){
        cout << number++ << ". " << item << '\n';
    }

    if (showBack){
        cout << "\n0. Назад\n";
    }

    cout << "\nВаш выбор: ";
    int choice;
    cin >> choice;
    return choice;
}

Algorithm chooseAlgorithm(){
    return static_cast<Algorithm>(
        showMenu(
            "Выберите алгоритм",
            {
                "Шифр Гронсфельда",
                "Шифр Зигзаг"
            }
        )
    );
}

void keyGeneratorMenu(){
    cout << "\nГенератор ключей\n\n";
    cout << "1. Шифр Гронсфельда\n";
    cout << "2. Шифр Зигзаг\n";
    cout << "\n0. Назад\n";
    cout << "Ваш выбор: ";
    int temp;
    cin >> temp;
    Algorithm choice = static_cast<Algorithm>(temp);

    switch (choice){
        case Algorithm::Gronsfeld:{
            string key = generateGronsfeldKey(8);
            cout << "\nКлюч Гронсфельда: " << key << endl;
            break;
        }
        case Algorithm::RailFence:{
            cout << "\nКоличество строк: " << generateRailFenceKey() << endl;
            break;
        }
        case Algorithm::Back:
            return;
        default:
            printError("Некорректный выбор.");
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
    cout << "\nВведите цифровой ключ: ";
    string key = readConsoleWordAscii();
    return processDLL("libgronsfeldcipher.so", data, encryptMode, key);
}

vector<unsigned char> processRailFence(const vector<unsigned char>& data, bool encryptMode){
    cout << "\nВведите количество строк: ";
    string key = readConsoleWordAscii();
    return processDLL("librailfencecipher.so", data, encryptMode, key);
}

void fileMenu(){
    Algorithm algorithm = chooseAlgorithm();
    if (algorithm == Algorithm::Back){
        return;
    }
    Mode mode = static_cast<Mode>(showMenu("Выберите режим",{
        "Шифрование",
        "Дешифрование"
        }));
    if (mode == Mode::Back){
        return;
    }
    cout << "\nВведите имя файла:\n";
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
            if (inputFile.rfind("bin/", 0) == 0) {
                outputFile = "bin/encrypted_" + inputFile.substr(4);
            } else {
                outputFile = "encrypted_" + inputFile;
            }
        }
        else {
        if (inputFile.rfind("bin/", 0) == 0) {
            outputFile = "bin/decrypted_" + inputFile.substr(4);
        } else {
            outputFile = "decrypted_" + inputFile;
        }
    }
        writeFile(outputFile, result);
        printSuccess("Файл сохранён как:\n" + outputFile);
    }
    catch (const exception& e) {
        printError(e.what());
    }
    waitForEnter();
}

void textMenu(){
    Algorithm algorithm = chooseAlgorithm();
    if (algorithm == Algorithm::Back){
        return;
    }
    Mode mode = static_cast<Mode>(
    showMenu("Выберите режим",{
            "Шифрование",
            "Дешифрование"
        }));
    if (mode == Mode::Back){
        return;
    }

    cin.ignore((numeric_limits<streamsize>::max)(), '\n');

    string text;
    if (mode == Mode::Encrypt){
        cout << "\nВведите текст:\n";
        text = readConsoleLineUtf8();
    }
    else{
        cout << "\nВведите HEX:\n";
        text = readConsoleLineUtf8();
    }

    if (text.empty()) {
        printError("Строка ввода пуста.");
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
            printError("Неверный HEX-формат.");
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
            cout << "\nРезультат (HEX):\n";
            char buf[3];
            for (unsigned char byte : result){
                snprintf(buf, 3, "%02X", byte);
                cout << buf;
            }
            cout << endl;
        }
        else{
            cout << "\nРезультат:\n";
            printConsoleUtf8(bytesToString(result));
        }
    }
    catch (const exception& e) {
        printError(e.what());
    }
    waitForEnter();
}

int main(){
    setlocale(LC_ALL, "");

    while (true){
        int choice = showMenu(
        "Система шифрования данных Sencipher",{
            "Работа с текстом",
            "Работа с файлами",
            "Генератор ключей"
        }, true);

        MainMenu menuChoice = static_cast<MainMenu>(choice);

        if (choice == 0 || menuChoice == MainMenu::Exit) {
            cout << "\nЗавершение программы.\n";
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
                printError("Некорректный выбор.");
                waitForEnter();
                break;
        }
    }
    return 0;
}
