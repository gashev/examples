#include <linux/module.h>
#include <linux/slab.h>

static char* param = NULL;

static int param_set_value(const char *buffer) {
    int size = strlen(buffer) + 1;

    param = kmalloc(sizeof(char*) * size, GFP_KERNEL);
    if (!param) {
        return -1;
    }

    strncpy(param, buffer, size);

    return 0;
}

static int param_set(const char *buffer, const struct kernel_param *kp) {
    if (param) {
        kfree(param);
    }

    return param_set_value(buffer);
}

static int param_get(char *buffer, const struct kernel_param *kp) {
    return sprintf(buffer, param);
}

static struct kernel_param_ops param_ops = {
    .set = param_set,
    .get = param_get,
};

module_param_cb(param, &param_ops, &param, 0644);
MODULE_PARM_DESC(param, "Parameter example.");

static int param_init(void) {
    if (!param) {
        return param_set_value("param");
    }

    return 0;
}

void param_exit(void) {
    kfree(param);
}

module_init(param_init);
module_exit(param_exit);

MODULE_AUTHOR("Oleg Gashev");
MODULE_LICENSE("GPL");
