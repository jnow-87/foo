#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := init.o kernel.o driver.o panic.o csection.o task.o sched/ fs/ memory/ binloader/

obj-$(CONFIG_KERNEL_SYSCALL) += syscall.o
obj-$(CONFIG_KERNEL_TIMER) += timer.o
obj-$(CONFIG_KERNEL_SIGNAL) += ksignal.o
obj-$(CONFIG_KERNEL_STAT) += stat.o
obj-$(CONFIG_KERNEL_TEST) += test.o
obj-$(CONFIG_KERNEL_PRINTF) += kprintf.o
obj-$(CONFIG_SC_SIGNAL) += usignal.o
obj-$(CONFIG_KERNEL_IPI) += ipi.o


####
# flags
####

## debug messages
init-cppflags-$(CONFIG_KERNEL_MSG_INIT) := -DBUILD_KERNEL_MSG_WARN \
										   -DBUILD_KERNEL_MSG_INFO

init-cppflags-$(CONFIG_KERNEL_NO_MSG_INIT) := -DBUILD_KERNEL_MSG_WARN_DISABLE \
											  -DBUILD_KERNEL_MSG_INFO_DISABLE

syscall-cppflags-$(CONFIG_SC_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
usignal-cppflags-$(CONFIG_SC_SIGNAL_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
