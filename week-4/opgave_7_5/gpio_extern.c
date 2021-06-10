/* External declaration */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h> // struct of_device_id
#include <linux/of.h> // of_match_ptr macro
#include <linux/ioport.h> // struct resource

static const struct of_device_id g_ids[] = {
 { .compatible = "gpio-extern", },
 { } // ends with empty; MUST be last member
};

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bedirhan Dincer");
MODULE_DEVICE_TABLE(of, g_ids);

static int gpio_ex_probe(struct platform_device* pdev);
static int gpio_ex_remove(struct platform_device* pdev);

extern struct platform_driver g_driver;

static int gpio_ex_probe(struct platform_device* pdev)
{
    printk(KERN_INFO "gpio_ex_probe(%s)\n", pdev->name);
    return 0;
}

static int gpio_ex_remove(struct platform_device* pdev)
{
    printk(KERN_INFO "gpio_ex_remove(%s)\n", pdev->name);
    return 0;
}

struct platform_driver g_driver = {
    .probe = gpio_ex_probe, // obliged
    .remove = gpio_ex_remove, // obliged
    // .shutdown // optional
    // .suspend // optional
    // .suspend_late // optional
    // .resume_early // optional
    // .resume // optional
    .driver = {
        .name = "gpio-extern", // name of the driver
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_ids), // compatible device id
    }
};

static int gpio_ex_init(void)
{
    int result;
    result = platform_driver_register(&g_driver);
    printk(KERN_INFO "gpio_ex_init() succesful\n");
    return result;
}

static void gpio_ex_exit(void)
{
    printk(KERN_INFO "gpio_ex_exit()\n");
    platform_driver_unregister(&g_driver);
}

module_init(gpio_ex_init);
module_exit(gpio_ex_exit);
