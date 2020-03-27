#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/livepatch.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "expression.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Patch calc kernel module");
MODULE_VERSION("0.1");

void livepatch_nop_cleanup(struct expr_func *f, void *c)
{
    /* suppress compilation warnings */
    (void) f;
    (void) c;
}

int livepatch_nop(struct expr_func *f, vec_expr_t args, void *c)
{
    (void) args;
    (void) c;
    pr_err("function nop is now patched\n");
    return 0;
}

/* clang-format off */
static struct klp_func funcs[] = {
    {
        .old_name = "user_func_nop",
        .new_func = livepatch_nop,
    },
    {
        .old_name = "user_func_nop_cleanup",
        .new_func = livepatch_nop_cleanup,
    },
    {},
};
static struct klp_object objs[] = {
    {
        .name = "calc",
        .funcs = funcs,
    },
    {},
};
/* clang-format on */

static struct klp_patch patch = {
    .mod = THIS_MODULE,
    .objs = objs,
};

static int livepatch_calc_init(void)
{
    int ret = klp_register_patch(&patch);
    if (ret)
        return ret;
    ret = klp_enable_patch(&patch);
    if (ret) {
        WARN_ON(klp_unregister_patch(&patch));
        return ret;
    }
    return 0;
}

static void livepatch_calc_exit(void)
{
    WARN_ON(klp_unregister_patch(&patch));
}

module_init(livepatch_calc_init);
module_exit(livepatch_calc_exit);
MODULE_INFO(livepatch, "Y");
