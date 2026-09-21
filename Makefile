CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

all: bindir bin/libgronsfeldcipher.so bin/librailfencecipher.so bin/main

bindir:
	mkdir -p bin

bin/libgronsfeldcipher.so: src/gronsfeldcipher.cpp include/gronsfeldcipher.h include/cipherapi.h
	$(CXX) $(CXXFLAGS) -fPIC -shared src/gronsfeldcipher.cpp -o bin/libgronsfeldcipher.so

bin/librailfencecipher.so: src/railfence.cpp include/railfence.h include/cipherapi.h
	$(CXX) $(CXXFLAGS) -fPIC -shared src/railfence.cpp -o bin/librailfencecipher.so

bin/main: src/main.cpp src/pluginloader.cpp src/keygenerator.cpp include/pluginloader.h include/keygenerator.h include/cipherapi.h
	$(CXX) $(CXXFLAGS) src/main.cpp src/pluginloader.cpp src/keygenerator.cpp -o bin/main -ldl

clean:
	rm -f bin/libgronsfeldcipher.so bin/librailfencecipher.so bin/main

.PHONY: all clean bindir
