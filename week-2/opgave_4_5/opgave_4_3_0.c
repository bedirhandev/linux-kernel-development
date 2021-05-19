#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");

static ssize_t hello_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t hello_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int hello_open(struct inode *inode, struct file *file);
static int hello_release(struct inode *inode, struct file *file);

#define MY_MAJOR  243
#define MY_MINOR  0
#define MY_VERSION 7
#define MY_COUNT 2
#define MY_NAME "opgave_4_3"
#define MY_SIZE 256
#define MY_DEVICE_NUMBER MKDEV(MY_MAJOR, MY_MINOR)

struct mem_dev
{
	char* data;
	unsigned long size;
};

struct mem_dev *mem_devp;
static struct cdev device[MY_COUNT];
static struct class *device_class = NULL;
static int buffer_size = 256;
static char buffer[256] = {0};
static int count = 0;

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
	printk(KERN_INFO "ppos %lld lbuf %ld buffer_size %d \n", *ppos, lbuf, MY_SIZE);
	//printk(KERN_INFO "%p", *file->private_data);

	//struct mem_dev *dev = file->private_data;

	if(*ppos >= MY_SIZE) 
	{
		return 0;
	}

	if(*ppos + lbuf > MY_SIZE)
	{
		// error
		printk(KERN_INFO "The number of bytes read can't go beyond the file size.\n");
		lbuf = MY_SIZE - (*ppos);
	}

	if(copy_to_user(buf, buffer + *ppos, lbuf) != 0) 
	{
		return -EIO;
	}

	*ppos += lbuf;
	return lbuf;
}

static ssize_t hello_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	printk(KERN_ALERT "hello_write()\n");
	printk(KERN_INFO "ppos %lld lbuf %ld buffer_size %d \n", *ppos, lbuf, MY_SIZE);

	//struct mem_dev *dev = file->private_data;

	if(*ppos >= MY_SIZE)
	{
		return -EINVAL;
	}

	if(*ppos + lbuf > MY_SIZE)
	{
		// error
		printk(KERN_ALERT "The number of bytes to write is too large.\n");
		lbuf = MY_SIZE - (*ppos);
	}

	/* copy data to buffer */
	if(copy_from_user(buffer + *ppos, buf, lbuf) != 0) 
	{
		return -EFAULT;
	}

	*ppos += lbuf;
	return lbuf;
}

static int hello_open(struct inode *inode, struct file *file)
{
	struct mem_dev *dev;

	int num = MINOR(inode->i_rdev);

	if(num >= MY_COUNT)
	{
		return -ENODEV;
	}

	dev = &mem_devp[num];
	file->private_data = dev;

	count++;
	//file->private_data = (void*) name;
	//printk(KERN_ALERT "%s\n", (char*) file->private_data);
	//file->private_data = (void*) age;
	//printk(KERN_ALERT "%d\n", (int) file->private_data);
	printk(KERN_ALERT "hello_open(). Open count: %d\n", count);
	return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT "hello_release()\n");
	return 0;
}

static int hello_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int hello_init(void)
{
	int i = 0;
	int err;

	err = register_chrdev_region(MY_DEVICE_NUMBER, MY_COUNT, MY_NAME);

	printk(KERN_ALERT "hello_init()\n");

	// -- check error of adding char device
	if (err < 0)
	{
		printk(KERN_INFO "hello_init() - driver (ver. %d) MY_MAJOR %d is not added.\n", MY_VERSION, MY_MAJOR);
		return -1;
	} else {
		printk("hello_init() - driver (ver. %d) MY_MAJOR %d is added.\n", MY_VERSION, MY_MAJOR);

		mem_devp = kvmalloc(MY_COUNT * sizeof(struct mem_dev), GFP_KERNEL);
		if(!mem_devp)
		{
			unregister_chrdev_region(MY_DEVICE_NUMBER, 1);
			return -ENOMEM;
		}

		memset(mem_devp, 0, sizeof(struct mem_dev));

		device_class = class_create(THIS_MODULE, MY_NAME);
		device_class->dev_uevent = hello_uevent;

		for(i = 0; i < MY_COUNT; i++)
		{
			//device[i] = cdev_alloc();
			//device[i].owner = THIS_MODULE;

			cdev_init(&device[i], &fops);
			device[i].owner = THIS_MODULE;

			err = cdev_add(&device[i], MKDEV(MY_MAJOR, i), 1); //MY_COUNT (3)
			if(err)
			{
				printk(KERN_INFO "WARNING");
			}
			mem_devp[i].size = MY_SIZE;
			mem_devp[i].data = kvmalloc(MY_SIZE, GFP_KERNEL);
			memset(mem_devp[i].data, 0, MY_SIZE);

			device_create(device_class, NULL, MKDEV(MY_MAJOR, i), NULL, "mychardev-%d", i);
		}
	}

	return 0;
}

static void hello_exit(void)
{
	int i = 0;
	printk(KERN_ALERT "hello_exit()\n");
	printk(KERN_ALERT "hello_exit() - driver (ver. %d) MY_MAJOR %d is removed.\n", MY_VERSION, MY_MAJOR);
	
	for(i = 0; i < MY_COUNT; i++)
	{
		device_destroy(device_class, MKDEV(MY_MAJOR, i));
		//cdev_del(&device[i]);
	}

	class_unregister(device_class);
	class_destroy(device_class);
	
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_COUNT);
	kvfree(mem_devp);
}

module_init(hello_init);
module_exit(hello_exit);
