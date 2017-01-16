#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    Expression expression;
    char variables[256];
    char output[4096];
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
        expression = calculate.create(argv[1], variables, errors);

        if (expression) {
            values = (double *) malloc((argc - 2) / 2 * sizeof(double));
            for (i = 3; i < argc; i += 2)
                values[(i + 1) / 2 - 2] = strtod(argv[i], NULL);

            result = calculate.evaluate(
                expression, values, (argc - 2) / 2, errors
            );

            if (!strcmp(errors, "")) {
                printf("Expression:\n");
                calculate.expression(expression, output);
                printf("%s\n", output);
                printf("Variables:\n");
                calculate.variables(expression, output);
                printf("%s\n", output);
                printf("Infix notation:\n");
                calculate.infix(expression, output);
                printf("%s\n", output);
                printf("Postfix notation:\n");
                calculate.postfix(expression, output);
                printf("%s\n", output);
                printf("Expression tree:\n");
                calculate.tree(expression, output);
                printf("%s\n", output);
                printf("Result:\n");
                printf("%f\n", result);
            }
            else
                printf("%s\n", errors);
        }
        else {
            printf("%s\n", errors);
        }
        calculate.free(expression);

    }

    return 0;
}
