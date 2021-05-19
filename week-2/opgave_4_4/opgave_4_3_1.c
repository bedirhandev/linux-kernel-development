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

#define MEMDEV_MAJOR 0 /*The default mem number of the mem*/
#define MEMDEV_NR_DEVS 2 /*Number of devices*/
#define MEMDEV_SIZE 256
#define MEMDEV_NAME "memdev"

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
    struct mem_dev *dev;
    
	/*Get the minor device number*/
    int num = MINOR(inode->i_rdev);
 
    if (num >= MEMDEV_NR_DEVS) return -ENODEV;
    dev = &mem_devp[num];
    
	/* Assign the device description structure pointer to the file private data pointer */ 
	filp->private_data = dev; //convenient to use the pointer later

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
	unsigned long p =  *ppos;
	unsigned int count = size;
	int ret = 0;
	struct mem_dev *dev = filp->private_data; /* get device structure pointer */

	/* Determine if the read position is valid*/
	if (p >= MEMDEV_SIZE) return 0; //Out of reading range, returning 0 means no data can be read
	
	if (count > MEMDEV_SIZE - p) count = MEMDEV_SIZE - p;

	/*Read data to user space*/
	if (copy_to_user(buf, (void*)(dev->data + p), count) != 0)
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
	unsigned long p =  *ppos;
	unsigned int count = size;
	int ret = 0;
	struct mem_dev *dev = filp->private_data; /* get device structure pointer */

	/* Analyze and get a valid write length*/
	if (p >= MEMDEV_SIZE) return 0;
	if (count > MEMDEV_SIZE - p) count = MEMDEV_SIZE - p;

	/*Write data from user space*/
	if (copy_from_user(dev->data + p, buf, count) != 0)
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
  .release = mem_release,
};

static int hello_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}
 
static int memdev_init(void)
{
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

	device_class = class_create(THIS_MODULE, MEMDEV_NAME);
	device_class->dev_uevent = hello_uevent;

	/* Initialize the cdev structure */
	cdev_init(&cdev, &mem_fops);//Connect cdev to mem_fops
	cdev.owner = THIS_MODULE; //owner member indicates who owns this driver, so that "kernel reference module count" is incremented by 1; THIS_MODULE indicates that this module is now used by the kernel, which is a kernel-defined macro
	cdev.ops = &mem_fops;

	/* Registered character device */
	cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);

	/* Allocate memory for device description structure */
	mem_devp = kvmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);//We have been using GFP_KERNEL so far
	if (!mem_devp) /* application failed */
	{
		result =  -ENOMEM;
		printk(KERN_ERR "Driver couldn't allocate memory for the device description structure.");
		goto fail_malloc;
	}

	memset(mem_devp, 0, sizeof(struct mem_dev));

	/* allocate memory for the device */
	for (i = 0; i < MEMDEV_NR_DEVS; i++) 
	{
		mem_devp[i].size = MEMDEV_SIZE;
		mem_devp[i].data = kvmalloc(MEMDEV_SIZE, GFP_KERNEL);//The assigned address exists here
		memset(mem_devp[i].data, 0, MEMDEV_SIZE);

		device_create(device_class, NULL, MKDEV(mem_major, i), NULL, "mychardev-%d", i);
	}

	printk(KERN_INFO "Driver succesfully initialized.");

	return 0;

	fail_malloc:
		unregister_chrdev_region(devno, 1);

	return result;
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
	kvfree(mem_devp); /*release device structure memory*/
	unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*release device number*/
	printk(KERN_INFO "Driver succesfully destroyed.");
}
 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

module_init(memdev_init);
module_exit(memdev_exit);
