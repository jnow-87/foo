#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := gpio.o

obj-$(CONFIG_AVR) += avr.o
obj-$(CONFIG_X86) += x86.o


cppflags-$(CONFIG_GPIO_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
