#include <iostream>

#include "calculate.h"

using namespace std;
using namespace calculate;


int main(int argc, char *argv[]) {
    Calculate exp1("1 + 2");
    Calculate exp2(exp1);
    Calculate exp3(move(Calculate("3 + 4")));

    cout << "exp1 == exp2: " << (exp1 == exp2) << endl;
    cout << "exp1 == exp3: " << (exp1 == exp3) << endl;
    cout << "exp2: " << exp2() << endl;
    cout << "exp3: " << exp3() << endl;

    return 0;
}
