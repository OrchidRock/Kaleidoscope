#include <iostream>

extern "C" {
    double fib(double);
}

int main(int argc, char *argv[])
{
        std::cout << "Fib(20): " << fib(20) << std::endl;
        return 0;
}
