i2c-gpio-param
==============

Installation
------------

1.  Modify KDIR variable in Makefile to point to your kernel sources.
2.  In case of a crosscompilation, pass ARCH and CROSS_COMPILE options to make command. For example:

   make ARCH=arm CROSS_COMPILE=arm-bcm2708-linux-gnueabi-

3.  Load resulting i2c-gpio-param.ko module.



Usage
-----

Linux kernel module for adding GPIO bitbanged I²C host device. 

It accepts following parameters:
  - `busid` (default 7) - I²C bus id (will create i2c-`busid` device)
  - `sda` (default 0) - GPIO pin number to be used as `SDA`
  - `scl` (default 1) - GPIO pin number to be used as `SCL`
  - `udelay` - SCL frequency is (500 / `udelay`)
  - `timeout` - clock streaching timeout
  - `sda_od` (bool) - SDA is configured as open drain
  - `scl_od` (bool) - SCL is configured as open drain
  - `scl_oo` (bool) - SCL output drivers cannot be turned of (no clock stretching)

You can add additional i2c-gpio busses or remove existing ones with sysfs interface:

  - `/sys/class/i2c-gpio/add_bus` accepts following space delimited integer arguments:

      busid ada scl [udelay] [timeout] [sda_od] [scl_od] [scl_oo]

  - `/sys/class/i2c-gpio/remove_bus` accepts one integer argument - busid

Example:

   # echo 5 2 3 > /sys/class/i2c-gpio/add_bus

   # echo 5 > /sys/class/i2c-gpio/remove_bus
