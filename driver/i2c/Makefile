#
# Copyright (C) 2019 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-$(CONFIG_I2C_PROTO_INT) += proto_int.o
obj-$(CONFIG_I2C_PROTO_POLL) += proto_poll.o
obj-$(CONFIG_I2C_DEV_INT) += dev_int.o
obj-$(CONFIG_I2C_DEV_POLL) += dev_poll.o
obj-$(CONFIG_AVR) += avr.o


cppflags-$(CONFIG_I2C_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
