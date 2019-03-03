#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/string.h> /* for memset() and memcpy() */

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

typedef enum { ADD = 0, SUB, MULT, DIV, POW, NONE } operation_t;

/* The prototype functions for the character driver */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static void prepare_calc(void);
static void calc(int first, int second, operation_t op);

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

    if (*msg_ptr == 0) {
        return 0;
    }

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

        if (error_count == 0) {
            return (size_of_message);
        } else {
            return -EFAULT;
        }
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
    size_of_message = 0;
    memset(message, 0, sizeof(char) * BUFF_SIZE);

    snprintf(message, BUFF_SIZE, "%s", buffer);
    size_of_message = strlen(message);
    printk(KERN_INFO "CALC: Received %d -> %s\n", size_of_message, message);

    if (size_of_message >= 3) {
        prepare_calc();
    }
    return len;
}

static void calc(int first, int second, operation_t op)
{
    switch (op) {
    case ADD: {
        result = first + second;
        break;
    }
    case SUB: {
        result = first - second;
        break;
    }
    case MULT: {
        result = first * second;
        break;
    }
    case DIV: {
        result = first / second;
        break;
    }
    case POW: {
        result = first ^ second;
        break;
    }
    default:
        result = 0;
        break;
    }
}

static void handle_case(int *first_num_stop, int *sec_num_start, int i)
{
    if (first_num_stop && *first_num_stop == 0) {
        *first_num_stop = i - 1;
    }

    if (sec_num_start && *sec_num_start == 0) {
        *sec_num_start = i + 1;
    }
}

static void prepare_calc(void)
{
    int first_num_stop = 0;
    int sec_num_start = 0;
    int op_idx = 0;
    operation_t op = NONE;

    for (int i = 0; i < BUFF_SIZE; i++) {
        if (message[i] == '\0') {
            break;
        }

        switch (message[i]) {
        case '*': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            op = MULT;
            break;
        }
        case '/': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            op = DIV;
            break;
        }
        case '+': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            op = ADD;
            break;
        }
        case '-': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            op = SUB;
            break;
        }
        case '^': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            op = POW;
            break;
        }
        case ' ': {
            handle_case(&first_num_stop, &sec_num_start, i);
            op_idx = i;
            break;
        }
        default:
            break;
        }
    }

    if (sec_num_start > 0 && op_idx > 0) {
        char **endp = NULL;
        int first_num = simple_strtol(message, endp, 10);
        int sec_num = simple_strtol((message + sec_num_start), endp, 10);

        printk(KERN_INFO "input is valid !\n");

        calc(first_num, sec_num, op);

        printk(KERN_INFO "first num: %d\n", first_num);
        printk(KERN_INFO "sec num: %d\n", sec_num);
    }
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "CALC: Device successfully closed\n");
    return 0;
}

static int __init calc_init(void)
{
    printk(KERN_INFO "CALC: Initializing the CALC LKM\n");

    // Try to dynamically allocate a major number for the device -- more
    // difficult but worth it
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "CALC failed to register a major number\n");
        return major;
    }
    printk(KERN_INFO "CALC: registered correctly with major number %d\n",
           major);

    // Register the device class
    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_class)) {  // Check for error and clean up if there is
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(
            char_class);  // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "CALC: device class registered correctly\n");

    // Register the device driver
    char_dev =
        device_create(char_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_dev)) {         // Clean up if there is an error
        class_destroy(char_class);  // Repeated code but the alternative is
                                    // goto statements
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(char_dev);
    }
    printk(KERN_INFO "CALC: device class created correctly\n");
    return 0;
}

static void __exit calc_exit(void)
{
    device_destroy(char_class, MKDEV(major, 0));  // remove the device
    class_unregister(char_class);                 // unregister the device class
    class_destroy(char_class);                    // remove the device class
    unregister_chrdev(major, DEVICE_NAME);        // unregister the major number
    printk(KERN_INFO "CALC: Goodbye!\n");
}

module_init(calc_init);
module_exit(calc_exit);
