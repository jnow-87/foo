#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# target
obj-y := \
	escape.o \
	memmock.o \
	string.o \
	ctype.o \
	list.o \
	list1.o \
	queue.o \
	stack.o \
	memblock.o \
	register.o \
	vector.o \
	devtree.o

obj-$(CONFIG_LIB_STREAM) += stream.o
obj-$(CONFIG_LIB_GETOPT) += getopt.o
obj-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o
obj-$(CONFIG_SYS_LINEBUFFER) += linebuf.o
obj-$(CONFIG_SYS_PATTERN_MATCH) += patmat.o
obj-$(CONFIG_KERNEL_NET) += inet.o

# host
hostobj-y := \
	escape.o \
	memmock.o \
	string.o \
	ctype.o \
	list.o \
	list1.o \
	queue.o \
	stack.o \
	memblock.o \
	register.o \
	vector.o \
	devtree.o \
	stream.o \
	getopt.o \
	ringbuf.o \
	linebuf.o \
	patmat.o \
	inet.o


# flags
cppflags-y := -DTEST_TYPE=\"unit\"
hostcppflags-y := $(cppflags-y)
