#include "expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int status = 0;

static float user_func_next(struct expr_func *f, vec_expr_t args, void *c)
{
    (void) f, (void) c;
    float a = expr_eval(&vec_nth(&args, 0));
    return a + 1;
}

static struct expr_func user_funcs[] = {
    {"next", user_func_next, NULL, 0},
    {NULL, NULL, NULL, 0},
};

static void test_benchmark(const char *s)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    double start = t.tv_sec + t.tv_usec * 1e-6;
    struct expr_var_list vars = {0};
    struct expr *e = expr_create(s, strlen(s), &vars, user_funcs);
    if (e == NULL) {
        printf("FAIL: %s can't be compiled\n", s);
        status = 1;
        return;
    }
    long N = 1000000L;
    for (long i = 0; i < N; i++) {
        expr_eval(e);
    }
    gettimeofday(&t, NULL);
    double end = t.tv_sec + t.tv_usec * 1e-6;
    expr_destroy(e, &vars);
    double ns = 1000000000 * (end - start) / N;
    printf("BENCH %40s:\t%f ns/op (%dM op/sec)\n", s, ns, (int) (1000 / ns));
}

int main()
{
    test_benchmark("5");
    test_benchmark("5+5+5+5+5+5+5+5+5+5");
    test_benchmark("5*5*5*5*5*5*5*5*5*5");
    test_benchmark("5,5,5,5,5,5,5,5,5,5");
    test_benchmark("((5+5)+(5+5))+((5+5)+(5+5))+(5+5)");
    test_benchmark("x=5");
    test_benchmark("x=5,x+x+x+x+x+x+x+x+x+x");
    test_benchmark("x=5,((x+x)+(x+x))+((x+x)+(x+x))+(x+x)");
    test_benchmark("a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10");
    test_benchmark("a=1,a=2,a=3,a=4,a=5,a=6,a=7,a=8,a=9,a=10");
    test_benchmark("$(sqr,$1*$1),5*5");
    test_benchmark("$(sqr,$1*$1),sqr(5)");
    test_benchmark("x=2+3*(x/(42+next(x))),x");
    test_benchmark("a,b,c,d,e,d,e,f,g,h,i,j,k");
    test_benchmark("$(a,1),$(b,2),$(c,3),$(d,4),5");

    return status;
}
