#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define DEVICE_NAME "test"

static char *msg = DEVICE_NAME;
static dev_t first;       // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl;  // Global variable for the device class

static ssize_t dev_read(
    struct file* F,
    char *buffer,
    size_t count,
    loff_t *f_pos
) {
    int bytes_read = 0;

    while (*msg && (bytes_read < count)) {
        put_user(*(msg++), buffer++);

        bytes_read++;
    }

    return bytes_read;
}

static struct file_operations FileOps = {
    .owner        = THIS_MODULE,
    .read         = dev_read,
};

static int dev_uevent(struct device *dev, struct kobj_uevent_env *env) {
    add_uevent_var(env, "DEVMODE=%#o", 0444);
    return 0;
}

static int init_chdev(void) {
    int init_result = alloc_chrdev_region(&first, 0, 1, DEVICE_NAME);

    if (init_result < 0) {
        printk(KERN_ALERT "Device Registration failed\n");
        return -1;
    }

    if ((cl = class_create(THIS_MODULE, "chardev")) == NULL) {
        printk(KERN_ALERT "Class creation failed\n");
        unregister_chrdev_region( first, 1);
        return -1;
    }

    cl->dev_uevent = dev_uevent;

    if (device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ALERT "Device creation failed\n");
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    cdev_init(&c_dev, &FileOps);

    if (cdev_add(&c_dev, first, 1) == -1) {
        printk(KERN_ALERT "Device addition failed\n" );
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    return 0;
}

void exit_chdev(void) {
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);

    printk(KERN_ALERT "Device unregistered\n");
}

module_init(init_chdev);
module_exit(exit_chdev);

MODULE_AUTHOR("Oleg Gashev");
MODULE_LICENSE("GPL");

