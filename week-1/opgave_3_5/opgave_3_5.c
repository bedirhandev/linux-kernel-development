#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

static ssize_t hello_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t hello_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int hello_open(struct inode *inode, struct file *file);
static int hello_release(struct inode *inode, struct file *file);

static struct cdev *device;

#define MY_MAJOR  243
#define MY_MINOR  0
#define MY_VERSION 5
#define MY_COUNT 1
#define MY_NAME "opgave_3_5"
#define MY_DEVICE_NUMBER MKDEV(MY_MAJOR, MY_MINOR)

struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write,
	.open = hello_open,
	.release = hello_release,
};

static ssize_t hello_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	printk(KERN_ALERT "hello_read()\n");
	return 0;
}

static ssize_t hello_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	printk(KERN_ALERT "hello_write()\n");
	return lbuf;
}

static int hello_open(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "hello_open()\n");
	return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "hello_release()\n");
	return 0;
}

static int hello_init(void)
{
	int err;

	device = cdev_alloc();
	device->owner = THIS_MODULE;

	register_chrdev_region(MY_DEVICE_NUMBER, MY_COUNT, MY_NAME);
	cdev_init(device, &fops);
	err = cdev_add(device, MY_DEVICE_NUMBER, MY_COUNT);

	printk(KERN_ALERT "hello_init()\n");

	// -- check error of adding char device
	if (err < 0)
	{
		printk("hello_init() - driver (ver. %d) MY_MAJOR %d is added.\n", MY_VERSION, MY_MAJOR);
		return -1;
	} else {
		printk("hello_init() - driver (ver. %d) MY_MAJOR %d is added.\n", MY_VERSION, MY_MAJOR);
	}

	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "hello_exit()\n");
	printk(KERN_ALERT "hello_exit() - driver (ver. %d) MY_MAJOR %d is removed.\n", MY_VERSION, MY_MAJOR);
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), 1);
	cdev_del(device);
}

module_init(hello_init);
module_exit(hello_exit);
