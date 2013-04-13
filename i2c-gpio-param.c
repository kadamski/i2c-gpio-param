#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>

#define MAX_BUSES 2

struct bus {
    int id;
    struct platform_device *pdev;
};

struct bus buses[MAX_BUSES];
static int n_buses;

static int addbus(unsigned int id, struct i2c_gpio_platform_data pdata);

static int busid=7;
static int sda=0;
static int scl=1;
static int udelay=0;
static int timeout=0;
static int sda_od=0;
static int scl_od=0;
static int scl_oo=0;

module_param(busid, int, S_IRUSR);
MODULE_PARM_DESC(busid, "I2C bus id");

module_param(sda, int, S_IRUSR);
MODULE_PARM_DESC(sda, "SDA pin");

module_param(scl, int, S_IRUSR);
MODULE_PARM_DESC(scl, "SCL pin");

module_param(udelay, int, S_IRUSR);
MODULE_PARM_DESC(udelay, "SCL frequency is (500 / udelay) kHz.");

module_param(timeout, int, S_IRUSR);
MODULE_PARM_DESC(timeout, "Clock stretching timeout in jiffies.");

module_param(sda_od, int, S_IRUSR);
MODULE_PARM_DESC(sda_od, "SDA is configured as open drain.");

module_param(scl_od, int, S_IRUSR);
MODULE_PARM_DESC(scl_od, "SCL is configured as open drain.");

module_param(scl_oo, int, S_IRUSR);
MODULE_PARM_DESC(scl_oo, "SCL output drivers cannot be turned off (no clock stretching).");

ssize_t sysfs_add_bus(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_gpio_platform_data pdata={};
    unsigned int id, ret, sdaod, sclod, scloo;
    char *blank;

    blank = strchr(buf, ' ');
    if (!blank) {
        dev_err(dev, "%s: Missing parameters\n", "add_bus");
        return -EINVAL;
    }

    ret=sscanf(buf, "%u %u %u %u %u %u %u %u", 
               &id, &pdata.sda_pin, &pdata.scl_pin, &pdata.udelay, &pdata.timeout, 
               &sdaod, &sclod, &scloo);
    if(ret<3) {
        dev_err(dev, "%s: Missing or wrong required parameters (busid, sda, scl).\n", "add_bus");
        return -EINVAL;
    }
    if(ret>5) {
        pdata.sda_is_open_drain=sdaod;
    }
    if(ret>6) {
        pdata.scl_is_open_drain=sclod;
    }
    if(ret>7) {
        pdata.scl_is_output_only=scloo;
    }
    printk(KERN_INFO "id=%u, sda=%u, scl=%u, udelay=%u, timeout=%u, sda_od=%u, scl_od=%u, scl_oo=%u\n", 
           id, pdata.sda_pin, pdata.scl_pin, pdata.udelay, pdata.timeout, pdata.sda_is_open_drain,
           pdata.scl_is_open_drain, pdata.scl_is_output_only);

    ret=addbus(id, pdata);
    if(ret) {
       return ret;
    }

    return count;
}

ssize_t sysfs_remove_bus(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static DEVICE_ATTR(add_bus, S_IWUSR | S_IRUGO, NULL, sysfs_add_bus);
static DEVICE_ATTR(remove_bus, S_IWUSR | S_IRUGO, NULL, sysfs_remove_bus);

static int addbus(unsigned int id, struct i2c_gpio_platform_data pdata)
{
    int ret;
    unsigned int i;
    struct platform_device *pdev;

    if(n_buses>=MAX_BUSES) {
        return -ENOMEM;
    }

    for(i=0; i<n_buses; i++) {
        if(buses[i].id==id)
            return -EEXIST;
    }

    pdev = platform_device_alloc("i2c-gpio", id);
    if (!pdev) {
         return -ENOMEM;
    }

    ret = platform_device_add_data(pdev, &pdata, sizeof(pdata));
    if(ret) {
        platform_device_put(pdev);
        return ret;
    }

    ret = platform_device_add(pdev);
    if(ret) {
        return ret;
    }
    // platform_device_add won't return error code if GPIO resources are not avaiable,
    // for example. A workaround is to check if drvdata is set after this function
    // as this is the last thing i2c_gpio_probe
    if(platform_get_drvdata(pdev)==NULL) {
        printk(KERN_ERR "i2c-gpio-param: Got error when registering the bus.\n");
        platform_device_unregister(pdev);
        return -EEXIST;
    }
    ret = device_create_file(&pdev->dev, &dev_attr_add_bus);
    if (ret) {
        platform_device_unregister(pdev);
        return ret;
    }

    ret = device_create_file(&pdev->dev, &dev_attr_remove_bus);
    if (ret) {
        platform_device_unregister(pdev);
        device_remove_file(&pdev->dev, &dev_attr_add_bus);
        return ret;
    }

    buses[n_buses++]=(struct bus){.id=id, .pdev=pdev};

    return 0;
}

static int __init i2c_gpio_param_init(void)
{
    struct i2c_gpio_platform_data pdata;
    int ret;

    pdata.sda_pin = sda;
    pdata.scl_pin = scl;
    pdata.udelay = udelay;
    pdata.timeout = timeout;
    pdata.sda_is_open_drain = sda_od;
    pdata.scl_is_open_drain = scl_od;
    pdata.scl_is_output_only = scl_oo;

    ret=addbus(busid, pdata);
    if(ret) {
        return ret;
    }

    return 0;
}

static void __exit i2c_gpio_param_exit(void)
{
    int i;
    for(i=0; i<n_buses; i++) {
        device_remove_file(&buses[i].pdev->dev, &dev_attr_add_bus);
        device_remove_file(&buses[i].pdev->dev, &dev_attr_remove_bus);
        platform_device_unregister(buses[i].pdev);
    }
}

module_init(i2c_gpio_param_init);
module_exit(i2c_gpio_param_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Krzysztof Adamski <k@japko.eu>");
MODULE_DESCRIPTION("I2C-GPIO Driver");
MODULE_VERSION("0.2");
