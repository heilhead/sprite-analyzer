#pragma once

#include <iostream>

namespace util {
    template<typename ...Args>
    void print(Args&& ...args) {
        (std::cout << ... << args) << std::endl;
    }

    template<typename ...Args>
    void printError(Args&& ...args) {
        (std::cerr << ... << args) << std::endl;
    }

    void bail(const char* message, int code = 1);
}