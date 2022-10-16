#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := kimg.elf

kimg-y := init.o kernel.o driver.o panic.o interrupt.o task.o sched/ fs/ memory/ binloader/ arch/ driver/ sys/ scripts/linker/kernel_$(CONFIG_ARCH).lds

kimg-$(CONFIG_KERNEL_SYSCALL) += syscall.o
kimg-$(CONFIG_KERNEL_TIMER) += timer/
kimg-$(CONFIG_KERNEL_SIGNAL) += ksignal.o
kimg-$(CONFIG_KERNEL_STAT) += stat.o
kimg-$(CONFIG_KERNEL_TEST) += test.o test/kernel/
kimg-$(CONFIG_KERNEL_PRINTF) += kprintf.o
kimg-$(CONFIG_KERNEL_IPI) += ipi.o
kimg-$(CONFIG_KERNEL_INTTASK) += inttask.o
kimg-$(CONFIG_KERNEL_NET) += net/
kimg-$(CONFIG_KERNEL_KTASK_QUEUE) += taskqueue.o
kimg-$(CONFIG_SC_SIGNAL) += usignal.o


####
# flags
####

kimg-cppflags-y += -DBUILD_KERNEL

kimg-ldlibs-y := \
	$(ldlibs-kernel) \
	-Lscripts/linker \
	-Tkernel_$(CONFIG_ARCH).lds \
	-lgcc

## debug messages
init-cppflags-$(CONFIG_KERNEL_MSG_INIT) := -DBUILD_KERNEL_MSG_FATAL \
										   -DBUILD_KERNEL_MSG_WARN \
										   -DBUILD_KERNEL_MSG_INFO \
										   -DBUILD_KERNEL_MSG_DEBUG

init-cppflags-$(CONFIG_KERNEL_NO_MSG_INIT) := -DBUILD_KERNEL_MSG_FATAL_DISABLE \
											  -DBUILD_KERNEL_MSG_WARN_DISABLE \
											  -DBUILD_KERNEL_MSG_INFO_DISABLE \
											  -DBUILD_KERNEL_MSG_DEBUG_DISABLE

syscall-cppflags-$(CONFIG_SC_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
usignal-cppflags-$(CONFIG_SC_SIGNAL_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
