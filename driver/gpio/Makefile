#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := gpio.o

obj-$(CONFIG_GPIO_PIN) += pin.o
obj-$(CONFIG_GPIO_PORT) += port.o
obj-$(CONFIG_GPIO_INTFWD) += intfwd.o


cppflags-$(CONFIG_GPIO_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
