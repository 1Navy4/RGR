#ifndef KEYGENERATOR_H
#define KEYGENERATOR_H

#include <string>

namespace SenCipher {
    std::string generateGronsfeldKey(int length);
    int generateRailFenceKey();
}

#endif
