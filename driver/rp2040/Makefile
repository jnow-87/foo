#
# Copyright (C) 2023 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-$(CONFIG_RP2040_UART) += uart.o
obj-$(CONFIG_RP2040_GPIO) += gpio.o
obj-$(CONFIG_RP2040_I2C) += i2c.o


i2c-cppflags += -DBUILD_KERNEL_LOG_DEBUG -O0
