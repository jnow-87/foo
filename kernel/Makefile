#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := init.o kernel.o panic.o csection.o task.o sched/ fs/ memory/ binloader/

obj-$(CONFIG_KERNEL_SYSCALL) += syscall.o
obj-$(CONFIG_KERNEL_TIMER) += timer.o
obj-$(CONFIG_KERNEL_SIGNAL) += signal.o
obj-$(CONFIG_KERNEL_STAT) += stat.o
obj-$(CONFIG_KERNEL_TEST) += test.o
obj-$(CONFIG_KERNEL_PRINTF) += kprintf.o


####
# flags
####

## avr
# disable jump tables if kernel is mapped to addresses larger than 64k
# this is required due to avr-gcc implementation of jump tables
ifeq ($(shell test $$(printf "%u" $(CONFIG_KERNEL_TEXT_BASE)) -gt $$(printf "%u" 0x10000); echo $$?),0)
  cflags-$(CONFIG_AVR) := -fno-jump-tables
endif

## debug messages
syscall-cppflags := -DBUILD_KERNEL_SYSCALL
init-cppflags := -DBUILD_KERNEL_INIT
