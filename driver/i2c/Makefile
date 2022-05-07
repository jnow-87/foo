#
# Copyright (C) 2019 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := i2c.o

obj-$(CONFIG_I2C_DEVFS_DEV) += dev.o
obj-$(CONFIG_AVR) += avr.o


cppflags-$(CONFIG_I2C_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
