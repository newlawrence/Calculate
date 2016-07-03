#include <stdio.h>

#include "calculate.h"


int main(int argc, char *argv[]) {
    CALC_Expression expr1, expr2, expr3;

    expr1 = CALC_newExpression("x + y", "x,y");
    expr2 = CALC_newExpression("x + &", "x,&");
    expr3 = CALC_newExpression("+", "");

    printf("%s\n", CALC_getExpression(expr1));
    printf("%s\n", CALC_getExpression(expr2));
    printf("%s\n", CALC_getExpression(expr3));

    CALC_freeExpression(expr1);
    CALC_freeExpression(expr2);
    CALC_freeExpression(expr3);

    return 0;
}
