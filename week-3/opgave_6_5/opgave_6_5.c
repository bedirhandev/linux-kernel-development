#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <asm/io.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

#define MEMDEV_MAJOR 0 /*The default mem number of the mem*/
#define MEMDEV_NR_DEVS 1 /*Number of devices*/
#define MEMDEV_SIZE 256
#define MEMDEV_NAME "memdev"
#define MEMDEV_CLASS "memdev-class"

static int mem_major = MEMDEV_MAJOR;
struct cdev cdev;
static struct class *device_class = NULL;

module_param(mem_major, int, S_IRUGO);

#define CM_MULT_GMPC_A3 0x44e1084c

#define GPIO1_ADDR 0x4804C000
#define GPIO1_START_ADDR 0x4804C000 //Physical address of GPIO
#define GPIO1_END_ADDR   0x4804e000 //Physical address of GPIO
#define GPIO1_SIZE (GPIO1_END_ADDR-GPIO1_START_ADDR)

#define CM_PER_BASE 		 0x44e00000
#define CM_PER_GPIO1		 0xac

/* 32-bits offset address */
#define GPIO_OE 0x4D
#define GPIO_DATAIN 0x4E
#define GPIO_CLEARDATAOUT 0x64
#define GPIO_SETDATAOUT 0x65
#define GPIO_MAX 0x198
/* define addresses. Is more convenient in underlaying code */
#define PIN 19

uint32_t* gpio1;
uint32_t oe;
uint32_t re;

static ssize_t dev_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t dev_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int dev_open(struct inode *inode, struct file *file);
static int dev_release(struct inode *inode, struct file *file);

struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_release,
};

static int dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	printk(KERN_INFO "dev_read()\n");
	return 0;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	printk(KERN_INFO "dev_write()\n");

	printk(KERN_INFO "initializing mux gpmc_a3..\n");

	uint32_t* cm_mult_gpmc_a3 = ioremap(CM_MULT_GMPC_A3, sizeof(*cm_mult_gpmc_a3)); // 4bytes
	if (cm_mult_gpmc_a3 == NULL ) return -EINVAL;
	barrier();
	uint32_t value = ioread32( cm_mult_gpmc_a3 );
	rmb();
	value &= (~0x7); // clear lower 3 bits
	value |= 0x7; // set lower 3 bits
	iowrite32( value, cm_mult_gpmc_a3 );
	wmb();
	iounmap(cm_mult_gpmc_a3);
	printk (KERN_INFO "Mux initialized.\n");
	barrier();

	/* output instellen */
	printk (KERN_INFO "Output enable for GPIO pin initializing..\n");
	gpio1 = ioremap( GPIO1_ADDR, GPIO_MAX ); 
	barrier();
	oe = ioread32( gpio1 + GPIO_OE );
	rmb();
	iowrite32( (oe & (~(1<<PIN))), gpio1 + GPIO_OE );
	wmb(); // write memory barrier
	iounmap(gpio1);
	printk (KERN_INFO "Output enable for GPIO pin initialized.\n");

	/* ledje aan en uit zetten */
	printk (KERN_INFO "Turning LED on/off with mux gpmc_a3 in mode 7..\n");
	gpio1 = ioremap(GPIO1_ADDR, GPIO_MAX); 
	barrier();
	iowrite32( (1<<PIN), gpio1 + GPIO_SETDATAOUT   ); // Pin 19 aan
	iowrite32( (1<<PIN), gpio1 + GPIO_CLEARDATAOUT ); // Pin 19 uit
	wmb(); // write memory barrier 
	iounmap(gpio1);
	printk (KERN_INFO "Turning LED on/off with mux gpmc_a3 in mode 7 successfull.\n");

	return lbuf;
}

static int dev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "dev_open()\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "dev_release()\n");
	return 0;
}

static int dev_init(void)
{
	printk(KERN_INFO "dev_init()\n");

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
	device_class->dev_uevent = dev_uevent;

	printk(KERN_INFO "Registering character device.");

	/* Initialize the cdev structure */
	cdev_init(&cdev, &fops);//Connect cdev to mem_fops
	cdev.owner = THIS_MODULE; //owner member indicates who owns this driver, so that "kernel reference module count" is incremented by 1; THIS_MODULE indicates that this module is now used by the kernel, which is a kernel-defined macro
	cdev.ops = &fops;

	curr_dev = MKDEV(MAJOR(devno), MINOR(devno));
	cdev_add(&cdev, curr_dev, MEMDEV_NR_DEVS);

	printk(KERN_INFO "Character device succesfully registered.");

	for (i = 0; i < MEMDEV_NR_DEVS; i++) 
	{
		printk(KERN_INFO "Device node: %d is being created.", i);
		curr_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
		device_create(device_class, NULL, curr_dev, NULL, "mychardev-%d", i);
		printk(KERN_INFO "Device node: %d is succesfully created.", i);
	}

	printk(KERN_INFO "Driver succesfully initialized.");

	return 0;
}

static void dev_exit(void)
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

	printk(KERN_INFO "dev_exit()\n");
}

module_init(dev_init);
module_exit(dev_exit);
