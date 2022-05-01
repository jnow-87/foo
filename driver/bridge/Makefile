#
# Copyright (C) 2021 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := bridge.o dgram.o

obj-$(CONFIG_BRIDGE_UART) += uart-dev.o uart-itf.o


cppflags-$(CONFIG_BRIDGE_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
