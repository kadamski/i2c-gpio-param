#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>

#define MAX_BUSES 8
#define MODNAME "i2c-gpio-param"

struct bus {
    int id;
    struct platform_device *pdev;
};

struct bus busses[MAX_BUSES];
static int n_busses;

static int addbus(unsigned int id, struct i2c_gpio_platform_data pdata);
static void removebus(unsigned int i);

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

static ssize_t add_bus_store(struct class *class,
                struct class_attribute *attr,
                const char *buf, size_t count)
{
    struct i2c_gpio_platform_data pdata={};
    unsigned int id, ret, sdaod, sclod, scloo;
    char *blank;

    blank = strchr(buf, ' ');
    if (!blank) {
        printk(KERN_INFO MODNAME " add_bus: Missing parameters\n");
        return -EINVAL;
    }

    ret=sscanf(buf, "%u %u %u %u %u %u %u %u", 
               &id, &pdata.sda_pin, &pdata.scl_pin, &pdata.udelay, &pdata.timeout, 
               &sdaod, &sclod, &scloo);
    if(ret<3) {
        printk(KERN_INFO MODNAME " add_bus: Missing or wrong required parameters (busid, sda, scl).\n");
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

static ssize_t remove_bus_store(struct class *class,
                struct class_attribute *attr,
                const char *buf, size_t count)
{
    unsigned int id,i;

    if(sscanf(buf, "%u", &id)<1) {
        printk(KERN_INFO MODNAME " remove_bus: Missing parameter.\n");
        return -EINVAL;
    }

    for(i=0; i<n_busses; i++) {
        if(busses[i].id==id) {
            removebus(i);
            if(i+1<n_busses)
                busses[i]=busses[n_busses-1];

            n_busses--;
            return count;
        }
    }

    return -ENOENT;
}


static struct class_attribute i2c_gpio_param_class_attrs[] = {
    __ATTR(add_bus, 0200, NULL, add_bus_store),
    __ATTR(remove_bus, 0200, NULL, remove_bus_store),
    __ATTR_NULL,
};

static struct class i2c_gpio_param_class = {
    .name =     "i2c-gpio",
    .owner =    THIS_MODULE,

    .class_attrs =  i2c_gpio_param_class_attrs,
};

static void removebus(unsigned int i) {
    platform_device_unregister(busses[i].pdev);
}

static int addbus(unsigned int id, struct i2c_gpio_platform_data pdata)
{
    int ret;
    unsigned int i;
    struct platform_device *pdev;

    if(n_busses>=MAX_BUSES) {
        return -ENOMEM;
    }

    for(i=0; i<n_busses; i++) {
        if(busses[i].id==id)
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
        printk(KERN_ERR MODNAME ": Got error when registering the bus.\n");
        platform_device_unregister(pdev);
        return -EEXIST;
    }

    busses[n_busses++]=(struct bus){.id=id, .pdev=pdev};

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

    ret = class_register(&i2c_gpio_param_class);
    if (ret < 0)
        return ret;


    return 0;
}

static void __exit i2c_gpio_param_exit(void)
{
    int i;

    class_unregister(&i2c_gpio_param_class);
    for(i=0; i<n_busses; i++) {
        removebus(i);
    }
}

module_init(i2c_gpio_param_init);
module_exit(i2c_gpio_param_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Krzysztof Adamski <k@japko.eu>");
MODULE_DESCRIPTION("I2C-GPIO Driver");
MODULE_VERSION("0.3");
