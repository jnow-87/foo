#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := kernel.o user.o page.o

obj-$(CONFIG_SC_DYNMEM) += syscall.o


cppflags-$(CONFIG_SC_DYNMEM_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
