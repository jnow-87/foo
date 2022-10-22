#
# Copyright (C) 2021 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := bridge.o dgram.o

obj-$(CONFIG_BRIDGE_UART_DEV) += uart-dev.o
obj-$(CONFIG_BRIDGE_UART_ITF) += uart-itf.o

obj-$(CONFIG_BRIDGE_I2C_DEV) += i2c.o i2c-dev.o
obj-$(CONFIG_BRIDGE_I2C_ITF) += i2c.o i2c-itf.o


cppflags-$(CONFIG_BRIDGE_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
