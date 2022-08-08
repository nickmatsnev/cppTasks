#include <iostream>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main() {
        while(1) {
        std::cout << "Hello world every 2 seconds" << std::endl;
        sleep(2);
    }
    return 0;
}