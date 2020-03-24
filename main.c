#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/string.h> /* for memset() and memcpy() */
#include "expression.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Math expression evaluation");
MODULE_VERSION("0.1");

#define DEVICE_NAME "calc"
#define CLASS_NAME "calc"
#define BUFF_SIZE 256

static int major;
static char message[BUFF_SIZE] = {0};
static short size_of_message;
static struct class *char_class = NULL;
static struct device *char_dev = NULL;
static char *msg_ptr = NULL;
static int result = 0;

/* The prototype functions for the character driver */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static void calc(void);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int dev_open(struct inode *inodep, struct file *filep)
{
    msg_ptr = message;
    return 0;
}

static ssize_t dev_read(struct file *filep,
                        char *buffer,
                        size_t len,
                        loff_t *offset)
{
    int error_count = 0;

    if (*msg_ptr == 0)
        return 0;

    memset(message, 0, sizeof(char) * BUFF_SIZE);

    snprintf(message, 64, "%d\n", result);
    size_of_message = strlen(message);

    error_count = copy_to_user(buffer, message, size_of_message);
    if (error_count == 0) {
        printk(KERN_INFO "CALC: size: %d result: %d\n", size_of_message,
               result);
        while (len && *msg_ptr) {
            error_count = put_user(*(msg_ptr++), buffer++);
            len--;
        }

        if (error_count == 0)
            return (size_of_message);
        return -EFAULT;
    } else {
        printk(KERN_INFO "CALC: Failed to send %d characters to the user\n",
               error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep,
                         const char *buffer,
                         size_t len,
                         loff_t *offset)
{
    memset(message, 0, sizeof(char) * BUFF_SIZE);

    if (len >= BUFF_SIZE) {
        printk(KERN_ALERT "Expression too long");
        return 0;
    }

    copy_from_user(message, buffer, len);
    printk(KERN_INFO "CALC: Received %ld -> %s\n", len, message);

    calc();
    return len;
}

static void calc(void)
{
    struct expr_var_list vars = {0};
    struct expr *e = expr_create(message, strlen(message), &vars, NULL);
    if (!e) {
        printk(KERN_ALERT "Syntax error");
        return;
    }

    result = expr_eval(e);
    printk(KERN_INFO "Result: %d\n", result);
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "CALC: Device successfully closed\n");
    return 0;
}

static int __init calc_init(void)
{
    printk(KERN_INFO "CALC: Initializing the CALC LKM\n");

    /* Try to dynamically allocate a major number for the device -- more
     * difficult but worth it
     */
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "CALC failed to register a major number\n");
        return major;
    }
    printk(KERN_INFO "CALC: registered correctly with major number %d\n",
           major);

    /* Register the device class */
    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_class)) {  // Check for error and clean up if there is
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(
            char_class); /* Correct way to return an error on a pointer */
    }
    printk(KERN_INFO "CALC: device class registered correctly\n");

    /* Register the device driver */
    char_dev =
        device_create(char_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_dev)) {        /* Clean up if there is an error */
        class_destroy(char_class); /* Repeated code but the alternative is
                                    *  goto statements
                                    */
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(char_dev);
    }
    printk(KERN_INFO "CALC: device class created correctly\n");
    return 0;
}

static void __exit calc_exit(void)
{
    device_destroy(char_class, MKDEV(major, 0)); /* remove the device */
    class_unregister(char_class);          /* unregister the device class */
    class_destroy(char_class);             /* remove the device class */
    unregister_chrdev(major, DEVICE_NAME); /* unregister the major number */
    printk(KERN_INFO "CALC: Goodbye!\n");
}

module_init(calc_init);
module_exit(calc_exit);
