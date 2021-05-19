#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

#define MEMDEV_MAJOR 243 /*The default mem number of the mem*/
#define MEMDEV_NR_DEVS 2 /*Number of devices*/
#define MEMDEV_SIZE 256
#define MEMDEV_NAME "memdev"
#define MEMDEV_CLASS "memdev-class"
static char* buffer;

/*mem device description structure*/
struct mem_dev                                     
{                                                        
  char *data;                      
  unsigned long size;       
};

static int mem_major = MEMDEV_MAJOR;
static int count = 0;
struct mem_dev *mem_devp; /* device structure pointer */
struct cdev cdev;
static struct class *device_class = NULL;

module_param(mem_major, int, S_IRUGO);
 
static ssize_t mem_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t mem_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int mem_open(struct inode *inode, struct file *file);
static int mem_release(struct inode *inode, struct file *file);

 /* file open function */
int mem_open(struct inode *inode, struct file *filp)
{
	filp->private_data = kvmalloc(MEMDEV_SIZE, GFP_KERNEL);//dev; //convenient to use the pointer later
	memset(filp->private_data, 0, MEMDEV_SIZE);

	count++;

	printk(KERN_ALERT "Device opened sucessfully. Open count: %d\n", count);
    
    return 0;
}
 
 /* file release function */
int mem_release(struct inode *inode, struct file *filp)
{
	kvfree(filp->private_data);
	printk(KERN_INFO "Device closed sucessfully.");
	return 0;
}
 
 /*Read function*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p =  *ppos;
	unsigned int count = size;
	int ret = 0;
	p = 0;

	/* Determine if the read position is valid*/
	if (p >= MEMDEV_SIZE) return 0; //Out of reading range, returning 0 means no data can be read
	
	//printk(KERN_INFO "%s", (dev->data + p));

	if (count > MEMDEV_SIZE - p) count = MEMDEV_SIZE - p;

	//char* buffer = filp->private_data;
	//printk(KERN_INFO "Content: %s\n", (char*)filp->private_data);
	/*char *t;
	for (t = (char*)filp->private_data; *t != '\0'; t++) {
		printk(KERN_INFO "Content: %c\n", *t);
	}*/

	//Read data to user space*/
	if (copy_to_user(buf, filp->private_data + p, count) != 0)
	{
		return  -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "Read %d bytes(s) from %ld\n", count, p);
	}

	return ret;
}
 
 /*Write function*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	printk(KERN_INFO "Writing started.");
	unsigned long p =  *ppos;
	unsigned int count = size;
	int ret = 0;

	/* Analyze and get a valid write length*/
	if (p >= MEMDEV_SIZE) return 0;
	if (count > MEMDEV_SIZE - p) count = MEMDEV_SIZE - p;

	//printk(KERN_INFO "Content: %p", (filp->private_data + p));

	/*Write data from user space*/
	if (copy_from_user(filp->private_data + p, buf, count) != 0)
	{
		return -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "Written %d bytes(s) from %ld\n", count, p);
	}
 
	return ret;
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

static int hello_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}
 
static int memdev_init(void)
{
	dev_t curr_dev;
	int result;
	int i;

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

	device_class = class_create(THIS_MODULE, MEMDEV_CLASS);
	device_class->dev_uevent = hello_uevent;

	printk(KERN_INFO "Registering character device.");

	/* Initialize the cdev structure */
	cdev_init(&cdev, &mem_fops);//Connect cdev to mem_fops
	cdev.owner = THIS_MODULE; //owner member indicates who owns this driver, so that "kernel reference module count" is incremented by 1; THIS_MODULE indicates that this module is now used by the kernel, which is a kernel-defined macro
	cdev.ops = &mem_fops;

	/* Registered character device */
	curr_dev = MKDEV(MAJOR(devno), MINOR(devno));
	cdev_add(&cdev, curr_dev, MEMDEV_NR_DEVS);

	printk(KERN_INFO "Character device succesfully registered.");

	/* allocate memory for the device */
	for (i = 0; i < MEMDEV_NR_DEVS; i++) 
	{
		printk(KERN_INFO "Device node: %d is being created.", i);
		curr_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
		device_create(device_class, NULL, curr_dev, NULL, "mychardev-%d", i);
		printk(KERN_INFO "Device node: %d is succesfully created.", i);
	}

	printk(KERN_INFO "Driver succesfully initialized.");
	printk(KERN_INFO "Driver succesfully initialized.");

	return 0;
}
 
 /* module unload function */
static void memdev_exit(void)
{
	int i = 0;

	for(i = 0; i < MEMDEV_NR_DEVS; i++)
	{
		device_destroy(device_class, MKDEV(mem_major, i));
	}

	class_unregister(device_class);
	class_destroy(device_class);

	cdev_del(&cdev); /*Logout device*/
	unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*release device number*/
	printk(KERN_INFO "Driver succesfully destroyed.");
}
 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

module_init(memdev_init);
module_exit(memdev_exit);
