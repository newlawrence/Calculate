#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "calculate.h"

using namespace std;
using namespace calculate;
using namespace calculate_exceptions;


int main(int argc, char *argv[]) {
    if (argc > 0 && argc % 2 == 0) {
        try {
            auto vars = vector<string>();
            auto values = vector<double>();
            for (auto i = 2; i < argc; i += 2) {
                vars.push_back(argv[i]);
                values.push_back(stod(argv[i + 1]));
            }

            ostringstream stream;
            auto expression = Expression(argv[1], vars);
            auto variables = String("");
            auto result = expression(values);
            if (expression.variables().size()) {
                copy(
                    expression.variables().begin(),
                    expression.variables().end(),
                    ostream_iterator<string>(stream, ",")
                );
                variables = stream.str().erase(stream.str().size() - 1, 1);
            }

            cout << "Expression:" << endl;
            cout << expression.expression() << endl;
            cout << "Variables:" << endl;
            cout << variables << endl;
            cout << "Infix notation:" << endl;
            cout << expression.infix() << endl;
            cout << "Postfix notation:" << endl;
            cout << expression.postfix() << endl;
            cout << "Expression tree:" << endl;
            cout << expression.tree() << endl;
            cout << "Result:" << endl;
            cout << result << endl;
        }
        catch (const BaseCalculateException &e) {
            cout << e.what() << endl;
        }
    }

    return 0;
}
