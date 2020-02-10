#include "expression.h"

#include <stdio.h>
#include <string.h>

/* Custom function that returns the sum of its two arguments */
static float add(struct expr_func *f, vec_expr_t args, void *c)
{
    float a = expr_eval(&vec_nth(&args, 0));
    float b = expr_eval(&vec_nth(&args, 1));
    return a + b;
}

static struct expr_func user_funcs[] = {
    {"add", add, 0},
    {NULL, NULL, 0},
};

int main()
{
    const char s[][100] = {"1&&0", "~567", "1/3", "1/3*6", "1.3|0"};
    // const char s[][100] = {"12773/ 31", "2+3", "4*5", "7-3", "-3", "99/2",
    // "100>>2", "100<<2", "1.234+5.423", "2.354-6.453"};
    for (int i = 0; i < 5; ++i) {
        struct expr_var_list vars = {0};
        struct expr *e = expr_create(s[i], strlen(s[i]), &vars, user_funcs);
        if (!e) {
            printf("Syntax error");
            return 1;
        }

        int result = expr_eval(e);
        int frac = result & 15;
        float real = (float) (result >> 4);
        if (frac & (1 << 4)) {
            frac = -((~frac & 15) + 1);
        }

        while (frac) {
            if (frac < 0) {
                ++frac;
                real /= 10;
            } else {
                --frac;
                real *= 10;
            }
        }

        printf("%s = %f\n", s[i], real);
        expr_destroy(e, &vars);
    }

    return 0;
}
