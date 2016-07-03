#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    CALC_Expression expression;
    char variables[256];

    if (argc == 2) {
        expression = CALC_newExpression(argv[1], "");
        if (CALC_getVariables(expression) >= 0)
            printf("%f\n", CALC_evaluate(expression));
    }

    else if (argc == 4) {
        strcpy(variables, argv[2]);
        expression = CALC_newExpression(argv[1], variables);
        if (CALC_getVariables(expression) >= 0)
            printf("%f\n", CALC_evaluate(expression, strtod(argv[3], NULL)));
    }

    else if (argc == 6) {
        strcpy(variables, argv[2]);
        strcat(variables, ",");
        strcat(variables, argv[4]);
        expression = CALC_newExpression(argv[1], variables);
        if (CALC_getVariables(expression) >= 0)
            printf("%f\n", CALC_evaluate(
                expression, strtod(argv[3], NULL), strtod(argv[5], NULL))
            );
    }

    return 0;
}
