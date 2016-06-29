#include <iostream>

#include "calculate.h"

using namespace std;
using namespace calculate;

int main(int argc, char *argv[]) {
    if (argc > 1)
        cout << evaluate(shuntingYard(tokenize(argv[1]))) << endl;
    return 0;
}
