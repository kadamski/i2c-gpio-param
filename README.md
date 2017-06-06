i2c-gpio-param
==============
Linux kernel module for adding GPIO bitbanged I²C host device. It uses i2c-gpio module
but adds a way to dynamicaly add and remove busses when kernel is load and using sysfs
interface. This means it can directly replace the existing hardware implementation which can be 
useful where the existing hardware contains bugs such as with the clock stretching bug in the 
BCM2837 used in the RaspberyPi. It can also be useful where more I²C hosts than hardware
peripherals are required.

It is designed for and tested on RaspberyPi but it should work (with possible minor changes like
pin_blacklist) on any other devices.

Installation
------------

0.  Ensure that you have CONFIG_I2c_GPIO enabled in your kernel.
1.  Modify KDIR variable in Makefile to point to your kernel sources.
2.  In case of a crosscompilation, pass ARCH and CROSS_COMPILE options to make command. For example:

   make ARCH=arm CROSS_COMPILE=arm-bcm2708-linux-gnueabi-

3.  Load resulting i2c-gpio-param.ko module.

Usage
-----

Linux kernel module for adding GPIO bitbanged I²C host device. 

It accepts following parameters:
  - `busid` (default 7) - I²C bus id (will create i2c-`busid` device)
  - `sda` (int, default 0) - GPIO pin number to be used as `SDA`
  - `scl` (int, default 1) - GPIO pin number to be used as `SCL`
  - `udelay` - (int, default 50 for scl_oo==1 or 5 otherwise) SCL frequency is (500KHz / `udelay`)
  - `timeout` - (int, default 100) clock stretching timeout in milliseconds 
  - `sda_od` (bool, default 0) - SDA is configured as open drain*
  - `scl_od` (bool, default 0) - SCL is configured as open drain*
  - `scl_oo` (bool, default 0) - SCL output drivers cannot be turned of (no clock stretching)

*Note that true open drain mode is not supported on RaspberyPi so please only use sxx_od=0 for open drain emulation

You can add additional i2c-gpio busses or remove existing ones with sysfs interface:

  - `/sys/class/i2c-gpio/add_bus` accepts following space delimited integer arguments:

      busid sda scl [udelay] [timeout] [sda_od] [scl_od] [scl_oo]

  - `/sys/class/i2c-gpio/remove_bus` accepts one integer argument - busid

Example:

   # echo 5 2 3 > /sys/class/i2c-gpio/add_bus - adds an i2c bitbanged device at busid 5 with sda on GPIO2 and scl on GPIO3

   # echo 5 > /sys/class/i2c-gpio/remove_bus - removes i2c bitbanged device at busid 5

You can type dmesg to check the i2c-gpio kernel module registration details.

For further details about the bit-banged i2c implementation behind this kernel driver, please refer to:
http://elixir.free-electrons.com/linux/latest/source/drivers/i2c/busses/i2c-gpio.c
