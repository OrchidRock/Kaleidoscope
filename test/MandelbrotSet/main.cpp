#include <iostream>

extern "C" {
    double mandel(double, double, double, double);
}

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

int main(int argc, char *argv[])
{
    mandel(-2.3, -1.3, 0.05, 0.07); 
    return 0;
}
