#include <string>
#include <vector>
#include <iostream>

#include "calculate.h"


int main(int argc, char *argv[]) {
    try {
        if (argc > 0 && argc % 2 == 0) {
            auto variables = std::vector<std::string>();
            auto values = std::vector<double>();
            for (auto i = 2; i < argc; i += 2) {
                variables.push_back(argv[i]);
                values.push_back(std::stod(argv[i + 1]));
            }

            calculate::Calculate expression(argv[1], variables);
            std::cout << expression(values) << std::endl;
        }
    }
    catch (const calculate_exceptions::BaseCalculateException &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
