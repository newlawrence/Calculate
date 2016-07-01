#include <iostream>

#include "calculate.h"

int main(int argc, char *argv[]) {
    if (argc > 1)
        std::cout << calculate::calculate(argv[1]) << std::endl;
    return 0;
}
