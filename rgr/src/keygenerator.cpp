#include "keygenerator.h"
#include <random>

using namespace std;

namespace SenCipher {
    string generateGronsfeldKey(int length) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, 9);

        string key;
        for (int i = 0; i < length; i++) {
            key += char('0' + dist(gen));
        }
        return key;
    }

    int generateRailFenceKey() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(2, 10);
        return dist(gen);
    }
}
