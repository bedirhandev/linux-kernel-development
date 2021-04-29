#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

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
