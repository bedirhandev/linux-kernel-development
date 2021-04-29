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
	printk(KERN_ALERT "hello_init()\n");

	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "hello_exit()\n");
}

module_init(hello_init);
module_exit(hello_exit);
