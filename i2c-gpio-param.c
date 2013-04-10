#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>

struct platform_device *pdev = NULL;

static int busid=7;
static int sda=0;
static int scl=1;
static int udelay=0;
static int timeout=0;
static int sda_od=0;
static int scl_od=0;
static int scl_oo=0;

module_param(busid, int, 0);
MODULE_PARM_DESC(sda, "I2C bus id");

module_param(sda, int, 0);
MODULE_PARM_DESC(sda, "SDA pin");

module_param(scl, int, 0);
MODULE_PARM_DESC(scl, "SCL pin");

module_param(udelay, int, 0);
MODULE_PARM_DESC(udelay, "SCL frequency is (500 / udelay) kHz.");

module_param(timeout, int, 0);
MODULE_PARM_DESC(timeout, "Clock stretching timeout in jiffies.");

module_param(sda_od, int, 0);
MODULE_PARM_DESC(sda_od, "SDA is configured as open drain.");

module_param(scl_od, int, 0);
MODULE_PARM_DESC(scl_od, "SCL is configured as open drain.");

module_param(scl_oo, int, 0);
MODULE_PARM_DESC(scl_oo, "SCL output drivers cannot be turned off (no clock stretching).");

static int __init i2c_gpio_param_init(void)
{
	struct i2c_gpio_platform_data pdata;
	int ret;

	pdev = platform_device_alloc("i2c-gpio", busid);
	if (!pdev) {
		return -ENOMEM;
	}

	pdata.sda_pin = sda;
	pdata.scl_pin = scl;
	pdata.udelay = udelay;
	pdata.timeout = timeout;
	pdata.sda_is_open_drain = sda_od;
	pdata.scl_is_open_drain = scl_od;
	pdata.scl_is_output_only = scl_oo;

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
    if(pdev) {
        platform_device_put(pdev);
    }
}

module_init(i2c_gpio_param_init);
module_exit(i2c_gpio_param_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Krzysztof Adamski <k@japko.eu>");
MODULE_DESCRIPTION("I2C-GPIO Driver");
MODULE_VERSION("0.1");
