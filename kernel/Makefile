#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := kimg.elf

kimg-y := init.o kernel.o driver.o panic.o interrupt.o task.o sched/ fs/ memory/ binloader/ arch/ driver/ sys/

kimg-$(CONFIG_KERNEL_SYSCALL) += syscall.o
kimg-$(CONFIG_KERNEL_TIMER) += timer/
kimg-$(CONFIG_KERNEL_SIGNAL) += ksignal.o
kimg-$(CONFIG_KERNEL_STAT) += stat.o
kimg-$(CONFIG_KERNEL_TEST) += test.o test/kernel/
kimg-$(CONFIG_KERNEL_LOG) += kprintf.o
kimg-$(CONFIG_KERNEL_IPI) += ipi.o
kimg-$(CONFIG_KERNEL_INTTASK) += inttask.o
kimg-$(CONFIG_KERNEL_NET) += net/
kimg-$(CONFIG_KERNEL_KTASK_QUEUE) += taskqueue.o
kimg-$(CONFIG_KERNEL_HEARTBEAT) += heartbeat.o
kimg-$(CONFIG_SC_SIGNAL) += usignal.o

linker_scripts := \
	$(build_tree)/scripts/linker/memory.lds \
	$(build_tree)/scripts/linker/kernel.i.lds

# ensure a rebuild if non-object-file dependencies are updated
kimg-y += $(linker_scripts)


####
# flags
####

kimg-cppflags-y += -DBUILD_KERNEL

kimg-ldlibs-y := \
	$(ldlibs-kernel) \
	$(addprefix -T,$(linker_scripts)) \
	-lgcc

## debug messages
ifeq ($(CONFIG_KERNEL_LOG_INIT),y)
init-cppflags-y := -DBUILD_KERNEL_LOG_FATAL \
				   -DBUILD_KERNEL_LOG_WARN \
				   -DBUILD_KERNEL_LOG_INFO \
				   -DBUILD_KERNEL_LOG_DEBUG
else
init-cppflags-y := -DBUILD_KERNEL_LOG_FATAL_DISABLE \
				   -DBUILD_KERNEL_LOG_WARN_DISABLE \
				   -DBUILD_KERNEL_LOG_INFO_DISABLE \
				   -DBUILD_KERNEL_LOG_DEBUG_DISABLE
endif

syscall-cppflags-$(CONFIG_SC_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
usignal-cppflags-$(CONFIG_SC_SIGNAL_DEBUG) := -DBUILD_KERNEL_LOG_DEBUG
