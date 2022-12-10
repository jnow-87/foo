#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := \
	core.o \
	interrupt.o \
	isr.o \
	start.o \
	syscall.o \
	thread.o

obj-$(CONFIG_KERNEL_TIMER) += timer.o
obj-$(CONFIG_IOVERFLOW_DET) += iovfl.o
