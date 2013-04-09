#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>

struct platform_device *pdev = NULL;

static int __init i2c_gpio_param_init(void)
{
	struct i2c_gpio_platform_data pdata;
	int ret;

	pdev = platform_device_alloc("i2c-gpio", 5);
	if (!pdev) {
		return -ENOMEM;
	}

	pdata.sda_pin = 0;
	pdata.scl_pin = 1;
	pdata.udelay = 0;
	pdata.timeout = 0;
	pdata.sda_is_open_drain = 0;
	pdata.scl_is_open_drain = 0;
	pdata.scl_is_output_only = 0;

    ret = platform_device_add_data(pdev, &pdata, sizeof(pdata));
    if(ret) {
        platform_device_put(pdev);
        return ret;
    }

	ret = platform_device_add(pdev);
    if(ret) {
        platform_device_put(pdev);
        return ret;
    }

	return 0;
}

static void __exit i2c_gpio_param_exit(void)
{
    printk(KERN_INFO "EXIT");
    if(pdev) {
        printk(KERN_INFO "PUT");
        platform_device_put(pdev);
    }
}

module_init(i2c_gpio_param_init);
module_exit(i2c_gpio_param_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Krzysztof Adamski <k@japko.eu>");
MODULE_DESCRIPTION("I2C-GPIO Driver");
MODULE_VERSION("0.1");
