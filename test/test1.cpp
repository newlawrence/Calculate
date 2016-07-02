#include <iostream>

#include "calculate.h"

int main(int argc, char *argv[]) {
    auto expression = calculate::Calculate("x + y + z", {"x", "y", "z"});
    return 0;
}
