#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := \
	atomic.o \
	escape.o \
	string.o \
	ctype.o \
	memblock.o \
	mutex.o \
	list1.o \
	list2.o \
	register.o \
	vector.o \
	devtree.o \
	scripts/devtree/

obj-$(CONFIG_KERNEL_LOG) += stream.o
obj-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o
obj-$(CONFIG_SYS_LINEBUFFER) += linebuf.o
obj-$(CONFIG_SYS_PATTERN_MATCH) += patmat.o
obj-$(CONFIG_KERNEL_NET) += inet.o
obj-$(CONFIG_FONTS) += font.o fonts/
obj-$(CONFIG_DRIVER_VRAM) += vram.o

# host
hostobj-y := \
	errno.o \
	escape.o \
	string.o \
	ctype.o \
	stream.o \
	memblock.o \
	ringbuf.o \
	linebuf.o \
	list1.o \
	list2.o \
	register.o \
	vector.o \
	devtree.o \
	scripts/devtree/ \
	patmat.o \
	inet.o

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := \
	atomic.o \
	errno.o \
	escape.o \
	string.o \
	ctype.o \
	list1.o \
	list2.o \
	register.o \
	vector.o \
	devtree.o \
	scripts/devtree/

libbrick-$(CONFIG_SC_DYNMEM) += memblock.o
libbrick-$(CONFIG_LIB_MUTEX) += mutex.o
libbrick-$(CONFIG_LIB_STREAM) += stream.o
libbrick-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o
libbrick-$(CONFIG_SYS_LINEBUFFER) += linebuf.o
libbrick-$(CONFIG_SYS_PATTERN_MATCH) += patmat.o
libbrick-$(CONFIG_KERNEL_NET) += inet.o
libbrick-$(CONFIG_FONTS) += font.o fonts/
libbrick-$(CONFIG_DRIVER_VRAM) += vram.o

libbrick-y := $(patsubst %.o,%.libbrick.o,$(libbrick-y))

# custom rules to generate separate objects files for src files that are required by kernel and libbrick
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libbrick.o,    $(loc_src_tree)/%.c,)


# libhost
hostlib-y += libhost.a

libhost-y := \
	list1.o \
	list2.o \
	register.o \
	string.o \
	vector.o \
	vram.o

#  libhost-hostcflags := -nostdinc
libhost-hostcppflags-y := -Iinclude
