/* External declaration */
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>

struct stm32_data {
    struct i2c_client* client;
};

static struct device *stm32_dev;

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

/* read */
static ssize_t stm32_myparam_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct stm32_data *ddata = platform_get_drvdata(pdev);
    int len;
    int ret;
    char read_data[3];

    len = sprintf(buf, "Slave address: %#x\n", ddata->client->addr);
    if (len <= 0)
        dev_err(&ddata->client->dev, "stm32: Invalid sprintf len: %d\n", len);

    ret = i2c_master_recv(ddata->client, read_data, 3);
    if(ret) pr_info("i2c_master_recv succesfull");
    else pr_info("i2c_master_recv failed (ret = %d)", ret);

    len = sprintf(buf, "Status LED: %s\n", read_data);

    return len;
}


/* write */
static ssize_t stm32_myparam_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct stm32_data *ddata = platform_get_drvdata(pdev);
    int ret;

    ret = i2c_master_send(ddata->client, buf, strlen(buf));
    if(ret) pr_info("i2c_master_send succesfull");
    else pr_info("i2c_master_send failed (ret = %d)", ret);

    return count;
}

static DEVICE_ATTR(myparam, S_IRUSR | S_IWUSR, stm32_myparam_show,
                stm32_myparam_store);

static struct attribute *stm32_attrs[] = {
    &dev_attr_myparam.attr,
    NULL
};

static const struct attribute_group stm32_attr_group = {
    .attrs = stm32_attrs,
};

static int stm32_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;

    pr_info("stm32_probe!\n");
    pr_info("\tSlave address: %#x\n", client->addr);
    pr_info("\tName: %s\n", client->name);
    pr_info("\tDriver: %s\n", (client->dev).driver->name);

    struct stm32_data *ddata = NULL;

    ddata = kzalloc(sizeof(struct stm32_data), GFP_KERNEL);
    if(ddata == NULL) {
        ret = -ENOMEM;
        goto err_op_failed;
    }

    ddata->client = client;
    i2c_set_clientdata(client, ddata);

    ret = sysfs_create_group(&client->dev.kobj, &stm32_attr_group);

    if(ret)
        goto err_op_failed;

    return 0;

    err_op_failed:
      kfree(ddata);
      return ret;  
}

static int stm32_remove(struct i2c_client *client)
{
    struct stm32_data *ddata;
    ddata = i2c_get_clientdata(client);
    sysfs_remove_group(&client->dev.kobj, &stm32_attr_group);
    kfree(ddata);

    printk(KERN_INFO "stm32_remove(%s)\n", client->name);
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