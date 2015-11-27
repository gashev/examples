#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_AUTHOR("Oleg Gashev");
MODULE_DESCRIPTION("kernel thread example");
MODULE_LICENSE("GPL");

int count = 3;
module_param(count, int, 0444);

char **text;
static struct task_struct **task;

static int test_thread(void *data) {
    set_freezable();
    allow_signal(SIGKILL);

    do {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(10);

        printk("%s\n", (char*) data);
    } while(!kthread_should_stop());

    return 0;
}

unsigned int get_count(const unsigned int n) {
    unsigned int size = 0;

    unsigned int tmp = n;

    do {
        size++;
        tmp = (tmp - tmp % 10) / 10;
    } while (tmp > 0);

    return size;
}

static int __init thread_init(void) {
    int i, j, size;

    if (count < 1) {
        printk(
            "`%d' invalid for parameter `count'. `count' must be > 0.\n",
            count
        );
        return -1;
    }

    task = kmalloc(sizeof(struct task_struct*) * count, GFP_KERNEL);
    if (!task) {
        return -ENOMEM;
    }

    text = kmalloc(sizeof(char*) * count, GFP_KERNEL);
    if (!text) {
        kfree(task);
        return -ENOMEM;
    }

    for(i = 0; i < count; i++) {
        size = 5 + get_count(i + 1);
        text[i] = kmalloc(sizeof(char*) * size, GFP_KERNEL);
        if (!text[i]) {
            kfree(task);
            for (j = 0; j < i; j++) {
                kfree(text[j]);
            }
            kfree(text);
            return -ENOMEM;
        }

        snprintf(text[i], size, "text%d", i + 1);
    }

    for(i = 0; i < count; i++) {
        task[i] = kthread_run(test_thread, text[i], text[i]);
        if (!task[i]) {
            for (j = 0; j < i; j++) {
                kthread_stop(task[j]);
            }

            kfree(task);

            for (i = 0; i < count; i++) {
                kfree(text[i]);
            }

            kfree(text);

            return -ENOMEM;
        }
    }

    return 0;
}

static void __exit thread_exit(void) {
    int i;

    for (i = 0; i < count; i++) {
        kthread_stop(task[i]);
    }

    kfree(task);

    for (i = 0; i < count; i++) {
        kfree(text[i]);
    }

    kfree(text);
}

module_init(thread_init);
module_exit(thread_exit);
