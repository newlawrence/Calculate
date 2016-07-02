#include <iostream>
#include <string>

#include "calculate.h"

using namespace std;
using namespace calculate;

int main(int argc, char *argv[]) {
    if (argc == 2) {
        auto expression = Calculate(argv[1]);
        cout << "expression: " << argv[1] << endl;
        cout << "result: " << expression() << endl;
    }
    else if (argc == 4) {
        auto expression = Calculate(argv[1], {argv[2]});
        cout << "expression: " << argv[1] << endl;
        cout << argv[2] << " = " << argv[3] << endl;
        cout << "result: " << expression(stod(argv[3])) << endl;
    }
    else if (argc == 6) {
        auto expression = Calculate(argv[1], {argv[2], argv[4]});
        cout << "expression: " << argv[1] << endl;
        cout << argv[2] << " = " << argv[3] << endl;
        cout << argv[4] << " = " << argv[5] << endl;
        cout << "result: " << expression(stod(argv[3]), stod(argv[5])) << endl;
    }
    else {
        auto expression = Calculate("x * y * z", {"x", "y", "z"});
        cout << "expression: x * y * z" << endl;
        cout << "x = 1" << endl;
        cout << "y = 2" << endl;
        cout << "z = 3" << endl;
        cout << expression({1, 2, 3}) << endl;
    }
    return 0;
}
