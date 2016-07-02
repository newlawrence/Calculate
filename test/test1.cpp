#include <iostream>
#include <string>
#include <vector>

#include "calculate.h"

using namespace std;
using namespace calculate;

int main(int argc, char *argv[]) {
    if (argc == 2) {
        auto expression = Calculate(argv[1]);
        cout << expression() << endl;
    }

    else if (argc == 4) {
        auto expression = Calculate(argv[1], {argv[2]});
        cout << expression(stod(argv[3])) << endl;
    }

    else if (argc == 6) {
        auto expression = Calculate(argv[1], {argv[2], argv[4]});
        cout << expression(stod(argv[3]), stod(argv[5])) << endl;
    }

    else if (argc > 6 && argc % 2 == 0) {
        auto variables = vector<string>();
        auto values = vector<double>();
        for (auto i = 2; i < argc; i += 2) {
            variables.push_back(argv[i]);
            values.push_back(stod(argv[i + 1]));
        }

        auto expression = Calculate(argv[1], variables);
        cout << expression(values) << endl;
    }

    return 0;
}
