#include <string>
#include <vector>
#include <iostream>

#include "calculate.h"

using namespace std;
using namespace calculate;


int main(int argc, char *argv[]) {
    try {
        if (argc == 2) {
            Calculate expression(argv[1]);
            cout << expression() << endl;
        }
        else if (argc > 2 && argc % 2 == 0) {
            auto variables = vector<string>();
            auto values = vector<double>();
            for (auto i = 2; i < argc; i += 2) {
                variables.push_back(argv[i]);
                values.push_back(stod(argv[i + 1]));
            }

            Calculate expression(argv[1], variables);
            cout << expression(values) << endl;
        }
    }
    catch (const BaseCalculateException &e) {
        cout << e.what() << endl;
    }

    return 0;
}
