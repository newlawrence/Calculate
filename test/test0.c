#include <stdio.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    Calculate expression;

    expression = newExpression("1 + 2", "");
    printf("%s\n", getExpression(expression));
    freeExpression(expression);

    return 0;
}
