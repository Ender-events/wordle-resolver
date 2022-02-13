#pragma once

#include <iostream>

class Debug {
};

template <typename T>
Debug& operator<<(Debug& os, const T& obj)
{
    std::cout << obj;
    static_cast<void>(obj);
    return os;
}
Debug debug {};
