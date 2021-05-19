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

#define MY_MAJOR 243
#define MY_MAX_MINORS 2
#define MEMDEV_SIZE 256

struct my_device_data {
  struct cdev cdev;
  char buffer[256];                      
  unsigned long size; 
};

struct my_device_data devs[MY_MAX_MINORS];

static ssize_t mem_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos);
static ssize_t mem_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos);
static int mem_open(struct inode *inode, struct file *file);
static int mem_release(struct inode *inode, struct file *file);
 
 /* file open function */
static int mem_open(struct inode *inode, struct file *filp)
{
    struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);
    filp->private_data = my_data;

    /* initialize device */

    printk(KERN_ALERT "Driver opened successfully.");
    
    return 0; 
}
 
 /* file release function */
int mem_release(struct inode *inode, struct file *filp)
{
  printk(KERN_ALERT "Driver released successfully.");
  return 0;
}
 
 /*Read function*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct my_device_data *my_data = (struct my_device_data *) filp->private_data;

  /* Determine if the read position is valid*/
  if (p >= MEMDEV_SIZE) //Out of reading range, returning 0 means no data can be read
    return 0;
  if (count > MEMDEV_SIZE - p)
    count = MEMDEV_SIZE - p;
 
     /*Read data to user space*/
  if (copy_to_user(buf, my_data->buffer + *ppos, count) != 0)
  {
    return  -EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;
    
    printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
  }
 
  return ret;
}
 
 /*Write function*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{

  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct my_device_data *my_data = (struct my_device_data *) filp->private_data;
  
     /* Analyze and get a valid write length*/
  if (p >= MEMDEV_SIZE)
    return 0;
  if (count > MEMDEV_SIZE - p)
    count = MEMDEV_SIZE - p;
    
     /*Write data from user space*/
  if (copy_from_user(my_data->buffer + p, buf, count) != 0)
  {
    return -EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;
    
    printk(KERN_INFO "written %d bytes(s) from %d\n", count, p);
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
 
static int memdev_init(void)
{
	int err, i;

  err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS, "my_device_driver");

  printk(KERN_ALERT "Device started initializing.");

  if(err != 0)
  {
    printk(KERN_ALERT "Initializing error.");
    return err;
  }

  for(i = 0; i < MY_MAX_MINORS; i++)
  {
    cdev_init(&devs[i].cdev, &mem_fops);
    cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
  }

  printk(KERN_ALERT "Device initialized succesfully");

  return 0;
}
 
 /* module unload function */
static void memdev_exit(void)
{
    int i;

    for(i = 0; i < MY_MAX_MINORS; i++)
    {
      cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS); /*release device number*/
}
 
MODULE_LICENSE("GPL");
 
module_init(memdev_init);
module_exit(memdev_exit);
