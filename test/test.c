#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    CALC_Expression expression;
    char variables[256];
    double *values;

    if (argc % 2 == 0) {
        if (argc == 2)
            strcpy(variables, "");
        else
            strcpy(variables, argv[2]);
        for (unsigned i = 4; i < argc; i += 2) {
            strcat(variables, ",");
            strcat(variables, argv[i]);
        }
        expression = CALC_newExpression(argv[1], variables);

        if (CALC_getVariables(expression) >= 0) {
            if (argc == 2) {
                printf("%f\n", CALC_evaluate(expression));
            }
            else {
                values = (double *) malloc((argc - 2) / 2 * sizeof(double));
                for (int i = 3; i < argc; i += 2)
                    values[(i + 1) / 2 - 2] = strtod(argv[i], NULL);
                printf(
                    "%f\n",
                    CALC_evalArray(expression, values, (argc - 2) / 2)
                );
            }
            CALC_freeExpression(expression);
        }
    }

    return 0;
}
