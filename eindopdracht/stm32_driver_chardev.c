// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#define DEVICE_NAME                 "stm32"
#define COMMAND_FRAME_START         't'
#define COMMAND_FRAME_END           's'
#define COMMAND_FRAME_LENGTH        3

// struct pointer representing a slave device connected to the bus
static struct i2c_client* slave_device;

static const struct i2c_device_id stm32_i2c_id[] = {
    { "stm32", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, stm32_i2c_id);

static const struct of_device_id stm32_of_match[] = {
    { .compatible = "mse,stm32" },
    { }
};

MODULE_DEVICE_TABLE(of, stm32_of_match);

static int stm32_open(struct inode *inode, struct file *file)  {
    int ret_val;
    char data_to_send[COMMAND_FRAME_LENGTH];

    data_to_send[0] = COMMAND_FRAME_START;
    data_to_send[1] = 30;                   // send dummy (invalid) command to verify connectivity
    data_to_send[2] = COMMAND_FRAME_END;
    ret_val = i2c_master_send(slave_device, data_to_send, COMMAND_FRAME_LENGTH);
    if (ret_val == COMMAND_FRAME_LENGTH)   {
        pr_info("stm32 successfully initialized.\n");
        return 0;
    }
    else {
        pr_info("stm32 could not be initialized.\n");
        return -1;
    }

    return 0;
}

static int stm32_close(struct inode *inode, struct file *file)  {
    pr_info("Called close for stm32 device.\n");
    return 0;
}

static long stm32_ioctl(struct file *file, unsigned int cmd, unsigned long arg)  {
    pr_info("Called ioctl for stm32 device: cmd = %d, arg = %ld\n", cmd, arg);
    return 0;
}

static ssize_t stm32_write(struct file* file, const char __user* buffer, size_t len, loff_t* offset)  {
    int ret_val;
    char data_to_send[COMMAND_FRAME_LENGTH];
    char local_buffer[COMMAND_FRAME_LENGTH - 2];

    if (copy_from_user(local_buffer, buffer, len)) {
        dev_err(&(slave_device->dev), "Could not copy buffer from user space\n");
        return -EFAULT;
    }

    data_to_send[0] = COMMAND_FRAME_START;
    data_to_send[1] = local_buffer[0];
    data_to_send[2] = COMMAND_FRAME_END;

    ret_val = i2c_master_send(slave_device, data_to_send, COMMAND_FRAME_LENGTH);
    if (ret_val == COMMAND_FRAME_LENGTH)   {
        pr_info("i2c_master_send successful");
    }
    else if (ret_val < 0)   {
        pr_info("i2c_master_send failed (ret_val = %d)", ret_val);
    }
    else    {
        pr_info("i2c_master_send wrote less than the expected bytes (ret_val = %d)", ret_val);
    }

    return ret_val - 2;
}

static ssize_t stm32_read(struct file* filep, char __user * buffer, size_t len, loff_t* offset)  {
    int ret_val;
    char read_data[COMMAND_FRAME_LENGTH];

    ret_val = i2c_master_recv(slave_device, read_data, COMMAND_FRAME_LENGTH);
    if (ret_val < 0)    {
        dev_warn(&(slave_device->dev), "Unable to read device: %d\n", ret_val);
        return ret_val;
    }

    if (read_data[0] != COMMAND_FRAME_START || read_data[2] != COMMAND_FRAME_END) {
        dev_warn(&(slave_device->dev), "Invalid received frame.\n");
        return -1;
    }

    if (copy_to_user(buffer, read_data, len)) {
        dev_err(&(slave_device->dev), "Could not copy buffer to user space\n");
        return -EFAULT;
    }

    return 0;
}

static const struct file_operations my_dev_fops = {
    // .owner = THIS_MODULE,
    .open = stm32_open,
    .read = stm32_read,
    .write = stm32_write,
    .release = stm32_close,
    .unlocked_ioctl = stm32_ioctl,
};

/*--------------------------------------------------------------------------------*/

static struct miscdevice stm32_miscdevice = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = DEVICE_NAME,
        .fops = &my_dev_fops,
};


static int stm32_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret_val;

    pr_info("stm32_probe!\n");
    pr_info("\tSlave address: %#x\n", client->addr);
    pr_info("\tName: %s\n", client->name);
    pr_info("\tDriver: %s\n", (client->dev).driver->name);

    ret_val = misc_register(&stm32_miscdevice);
    if (ret_val != 0) {
        pr_err("Could not register device %s\n", DEVICE_NAME);
        return ret_val;
    }
    pr_info("Device %s - assigned minor: %i\n", DEVICE_NAME, stm32_miscdevice.minor);

    slave_device = client;

    return 0;
}

static int stm32_remove(struct i2c_client *client)
{
    pr_info("stm32_remove!\n");
    misc_deregister(&stm32_miscdevice);
    return 0;
}


static struct i2c_driver stm32_i2c_driver = {
    .driver = {
        .name = "stm32_driver",
        .of_match_table = stm32_of_match,
    },
    .probe = stm32_probe,
    .remove = stm32_remove,
    .id_table = stm32_i2c_id
};

module_i2c_driver(stm32_i2c_driver);

MODULE_AUTHOR("Bedirhan Dincer <bedirhan_dincer@hotmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for a custom I2C communication with an stm32 device");