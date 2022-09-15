#
# Copyright (C) 2018 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := interactive.o non-interactive.o
hostobj-y := $(obj-y)

interactive-y := itty.o exit.o
interactive-$(CONFIG_DRIVER_GPIO) += gpioint.o
interactive-$(CONFIG_DRIVER_I2C) += i2c.o

non-interactive-y := \
	$(build_tree)/sys/devicetree.o \
	close.o \
	thread.o \
	stat.o \
	signal.o \
	tty.o \
	sleep.o \
	time.o \
	mmap.o

non-interactive-$(CONFIG_DRIVER_GPIO) += gpio.o
non-interactive-$(if $(CONFIG_X86),n,y) += sigthread.o kill.o


# flags
interactive-cppflags := -DTEST_TYPE=\"user_interactive\"
interactive-hostcppflags := $(interactive-cppflags)

non-interactive-cppflags := -DTEST_TYPE=\"user_noninteractive\"
non-interactive-hostcppflags := $(non-interactive-cppflags)
