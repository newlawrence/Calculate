#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    calculate_Expression expression;
    char variables[256];
    char errors[64];
    double *values;
    double result;
    int i;

    if (argc % 2 == 0) {
        if (argc == 2)
            strcpy(variables, "");
        else
            strcpy(variables, argv[2]);
        for (i = 4; i < argc; i += 2) {
            strcat(variables, ",");
            strcat(variables, argv[i]);
        }
        expression = calculate->createExpression(argv[1], variables, errors);

        if (expression) {
            values = (double *) malloc((argc - 2) / 2 * sizeof(double));
            for (i = 3; i < argc; i += 2)
                values[(i + 1) / 2 - 2] = strtod(argv[i], NULL);

            result = calculate->evaluateArray(
                expression, values, (argc - 2) / 2, errors
            );

            if (!strcmp(errors, ""))
                printf("%f\n", result);
            else
                printf("%s\n", errors);

            calculate->freeExpression(expression);
        }
        else
            printf("%s\n", errors);
    }

    return 0;
}
