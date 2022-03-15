#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"

/* MAX_LENGTH is set to 92 because
 * ssize_t can't fit the number > 92
 */
#define MAX_LENGTH 1093

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;
static DEFINE_MUTEX(fib_mutex);

#define BIGN
#ifdef BIGN

#define BIGNSIZE 12

typedef struct BigN {
    unsigned long long cell[BIGNSIZE];
} ubig;

#define init_ubig(x) for (int i = 0; i < BIGNSIZE; x.cell[i++] = 0)

#define P10_UINT64 10000000000000000000ULL
static ubig fib_sequence_ubig(long long k)
{
    if (k <= 1LL) {
        ubig ret;
        init_ubig(ret);
        ret.cell[0] = (unsigned long long) k;
        return ret;
    }

    ubig a, b, c;
    init_ubig(a);
    init_ubig(b);
    b.cell[0] = 1;

    for (int i = 2; i <= k; i++) {
        for (int j = 0; j < BIGNSIZE; j++)
            c.cell[j] = a.cell[j] + b.cell[j];

        for (int j = 0; j < BIGNSIZE - 1; j++) {
            // if lower overflows or lower >= 10^9, add a carry to upper
            if ((c.cell[j] < a.cell[j]) || (c.cell[j] >= P10_UINT64)) {
                c.cell[j] -= P10_UINT64;
                c.cell[j + 1] += 1;
            }
        }

        a = b;
        b = c;
    }

    return c;
}

size_t ubig_to_string(char *buf, size_t bufsize, const ubig *a)
{
    int index = BIGNSIZE - 1;
    while (!a->cell[index])
        index--;
    if (index == -1) {
        buf[0] = '0';
        buf[1] = '\0';
        return (size_t) 1;
    }

    char buf2[22];
    snprintf(buf2, bufsize, "%llu", a->cell[index--]);
    strcat(buf, buf2);
    while (index >= 0) {
        snprintf(buf2, bufsize, "%019llu", a->cell[index--]);
        strcat(buf, buf2);
    }
    return strlen(buf);
}
#else
static long long fib_sequence(long long k)
{
    /* FIXME: C99 variable-length array (VLA) is not allowed in Linux kernel. */
    long long f[k + 2];

    f[0] = 0;
    f[1] = 1;

    for (int i = 2; i <= k; i++) {
        f[i] = f[i - 1] + f[i - 2];
    }

    return f[k];
}
#endif

static int fib_open(struct inode *inode, struct file *file)
{
    if (!mutex_trylock(&fib_mutex)) {
        printk(KERN_ALERT "fibdrv is in use");
        return -EBUSY;
    }
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&fib_mutex);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
#ifdef BIGN
#define BUFFSIZE 500
    char fibbuf[BUFFSIZE] = "";
    ubig fib = fib_sequence_ubig(*offset);
    size_t len = ubig_to_string(fibbuf, BUFFSIZE, &fib);
    len = (size > len) ? (len + 1) : size;
    copy_to_user(buf, fibbuf, len);
    return (ssize_t) len;
#else
    return (ssize_t) fib_sequence(*offset);
#endif
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    return 1;
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    mutex_init(&fib_mutex);

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    fib_cdev->ops = &fib_fops;
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    mutex_destroy(&fib_mutex);
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
