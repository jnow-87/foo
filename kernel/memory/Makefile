#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := kernel.o page.o

obj-$(CONFIG_SC_DYNMEM) += syscall.o
obj-$(CONFIG_KERNEL_MEMCHECK) += check.o


cppflags-$(CONFIG_SC_DYNMEM_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
