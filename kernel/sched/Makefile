#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := thread.o process.o sched.o

obj-$(CONFIG_SC_SCHED) += syscall.o


cppflags-$(CONFIG_SC_SCHED_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
