#include "railfence.h"
#include <stdexcept>

using namespace std;

vector<unsigned char> CipherRailFence::encrypt(
    const vector<unsigned char>& data,
    const string& key)
{
    if (data.empty())
        return data;

    int numRails;

    try{
        numRails = stoi(key);
    }
    catch (...){
        throw invalid_argument("Ключ должен быть числом");
    }

    if (numRails <= 1){
        throw invalid_argument("Количество строк должно быть больше 1");
    }

    vector<vector<unsigned char>> fence(numRails);

    int rail = 0;
    bool directionDown = false;

    for (unsigned char ch : data){
        fence[rail].push_back(ch);

        if (rail == 0 || rail == numRails - 1){
            directionDown = !directionDown;
        }

        rail += directionDown ? 1 : -1;
    }

    vector<unsigned char> result;

    for (int i = 0; i < numRails; i++){
        result.insert(
            result.end(),
            fence[i].begin(),
            fence[i].end()
        );
    }

    return result;
}

vector<unsigned char> CipherRailFence::decrypt(
    const vector<unsigned char>& data,
    const string& key)
{
    if (data.empty())
        return data;

    int numRails;

    try{
        numRails = stoi(key);
    }
    catch (...){
        throw invalid_argument("Ключ должен быть числом");
    }

    if (numRails <= 1){
        throw invalid_argument("Количество строк должно быть больше 1");
    }

    vector<vector<bool>> marker(
        numRails,
        vector<bool>(data.size(), false)
    );

    int rail = 0;
    bool directionDown = false;

    for (size_t i = 0; i < data.size(); i++){
        marker[rail][i] = true;

        if (rail == 0 || rail == numRails - 1){
            directionDown = !directionDown;
        }

        rail += directionDown ? 1 : -1;
    }

    vector<vector<unsigned char>> fence(numRails);

    size_t index = 0;

    for (int i = 0; i < numRails; i++){
        for (size_t j = 0; j < data.size(); j++){
            if (marker[i][j]){
                fence[i].push_back(data[index++]);
            }
        }
    }

    vector<unsigned char> result;
    vector<size_t> railIndex(numRails, 0);

    rail = 0;
    directionDown = false;

    for (size_t i = 0; i < data.size(); i++){
        result.push_back(
            fence[rail][railIndex[rail]++]
        );

        if (rail == 0 || rail == numRails - 1){
            directionDown = !directionDown;
        }

        rail += directionDown ? 1 : -1;
    }

    return result;
}

CipherAPI* createCipher(){
    return new CipherRailFence();
}

void releaseCipher(CipherAPI* cipher){
    delete cipher;
}