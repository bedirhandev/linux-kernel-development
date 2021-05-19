#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

#define MEMDEV_MAJOR 0 /*The default mem number of the mem*/
#define MEMDEV_NR_DEVS 2 /*Number of devices*/
#define MEMDEV_SIZE 256
#define MEMDEV_NAME "memdev"

static int mem_major = MEMDEV_MAJOR;
static int count = 0;
struct cdev cdev;
static int buffer_size = 256;
static char buffer[256] = {0};

module_param(mem_major, int, S_IRUGO);
 
static ssize_t mem_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t mem_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int mem_open(struct inode *inode, struct file *file);
static int mem_release(struct inode *inode, struct file *file);

 /* file open function */
int mem_open(struct inode *inode, struct file *filp)
{
	count++;
	printk(KERN_ALERT "Device opened sucessfully. Open count: %d\n", count);
    
    return 0;
}
 
 /* file release function */
int mem_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "Device closed sucessfully.");
	return 0;
}
 
 /*Read function*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	int result = 0;
	int bytes_read = 0;

	if(*ppos + size > buffer_size)
	{
		// error
		printk(KERN_INFO "System wants to read more that it is able to. Adjusting the size to read.\n");
		size = buffer_size - (*ppos);
	}

	result = copy_to_user(buf, buffer + *ppos, size);
	bytes_read = size - result;

	*ppos += bytes_read;
	return bytes_read;
}
 
 /*Write function*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	int result = 0;
	int bytes_written = 0;

	printk(KERN_ALERT "%s", buf);

	if(*ppos + size > buffer_size)
	{
		// error
		printk(KERN_ALERT "Buffer overflow\n");
	}

	/* copy data to buffer */
	result = copy_from_user(buffer + *ppos, buf, size);
	bytes_written = size - result;
	*ppos += bytes_written;
	
	printk(KERN_ALERT "mem_write()\n");
	return bytes_written;
}

 /* file operation structure */
static const struct file_operations mem_fops =
{
  .owner = THIS_MODULE,
  .read = mem_read,
  .write = mem_write,
  .open = mem_open,
  .release = mem_release
};

static int memdev_init(void)
{
	dev_t curr_dev;
	int result;

	dev_t devno = MKDEV(mem_major, 0);

	/* Static application device number*/
	if (mem_major) {
		printk(KERN_INFO "Statically assigned with device number: %d.", mem_major);
		result = register_chrdev_region(devno, 2, MEMDEV_NAME);
	}
	else /* dynamically assigns the device number */
	{
		result = alloc_chrdev_region(&devno, 0, 2, MEMDEV_NAME);
		mem_major = MAJOR(devno);
		printk(KERN_INFO "Dynamically assigned with device number: %d.", mem_major);
	}

	if (result < 0) {
		printk(KERN_ALERT "Something wrong happened.");
		return result;
	}

	printk(KERN_INFO "Registering character device.");

	/* Initialize the cdev structure */
	cdev_init(&cdev, &mem_fops);//Connect cdev to mem_fops
	cdev.owner = THIS_MODULE; //owner member indicates who owns this driver, so that "kernel reference module count" is incremented by 1; THIS_MODULE indicates that this module is now used by the kernel, which is a kernel-defined macro
	cdev.ops = &mem_fops;

	curr_dev = MKDEV(MAJOR(devno), MINOR(devno));
	cdev_add(&cdev, curr_dev, MEMDEV_NR_DEVS);

	printk(KERN_INFO "Character device succesfully registered.");

	printk(KERN_INFO "Driver succesfully initialized.");

	return 0;
}
 
 /* module unload function */
static void memdev_exit(void)
{
	cdev_del(&cdev); /*Logout device*/
	unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*release device number*/
	printk(KERN_INFO "Driver succesfully destroyed.");
}
 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

module_init(memdev_init);
module_exit(memdev_exit);
