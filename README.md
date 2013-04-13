i2c-gpio-param
==============

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
