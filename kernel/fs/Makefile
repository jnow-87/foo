#
# Copyright (C) 2017 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := fs.o rootfs.o

obj-$(CONFIG_KERNEL_DEVFS) += devfs.o
obj-$(CONFIG_SC_FILESYSTEM) += syscall.o
obj-$(CONFIG_KERNEL_NET) += netfs.o


cppflags-$(CONFIG_SC_FILESYSTEM_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
