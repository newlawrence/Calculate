#include <iostream>

#include "calculate.hpp"


int main(int argc, char* argv[]) {
    if (argc > 1) {
        auto lexer = calculate::make_lexer<std::complex<double>>();
        auto tokens = lexer->tokenize(argv[1]);
        while (!tokens.empty()) {
            auto token = tokens.front();
            tokens.pop();
            std::cout << static_cast<int>(token.second) << " : ";
            std::cout << token.first << std::endl;
        }
    }
    return 0;
}
