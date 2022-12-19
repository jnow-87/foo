#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := \
	errno.o \
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
	devicetree.o

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
	devicetree.o \
	patmat.o \
	inet.o

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := \
	errno.o \
	escape.o \
	string.o \
	ctype.o \
	list1.o \
	list2.o \
	register.o \
	vector.o \
	devtree.o \
	devicetree.o

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


####
## device tree
####

dtc := $(build_tree)/scripts/devtree/dtc

devtree_preproc := $(loc_build_tree)/devicetree.i.dts
devtree_make := $(loc_build_tree)/devicetree.make
devtree_header := $(loc_build_tree)/devicetree.h
devtree_src := $(loc_build_tree)/devicetree.c

prepare_deps: $(devtree_header) $(devtree_make)

# include devicetree.make if it already exists
#
#	$(call include_devicetree)
#
#
# NOTE use this function over the regular make "include" directive to prevent make
# 	   from trying to create devicetree.make since it requires the config-header
# 	   which, if created as prerequisite for devtree would force the devtree build-
# 	   flags upon all of its dependencies, causing the brickos types.h to be
# 	   included instead of the host types.h, which causes a circular dependency on
# 	   the config-header since brickos' types.h requires it
define include_devicetree
	$(if $(wildcard $(build_tree)/sys/devicetree.make), \
		$(eval include $(build_tree)/sys/devicetree.make) \
		, \
	)
endef

# compile the desired devicetree <output-format>
#
#	$(call compile_devicetree,<output-format>,<sections>
#
#
# NOTE to prevent rebuilds of all files depending on the devicetree header, first compile
# 	   a temporary one and only update the actual header if the content changed
# NOTE Due to the temporary file, make will attempt to rebuild the actual devicetree header
# 	   on each of the build system stages if the devicetree script change without causing a
# 	   change in the devicetree header. To prevent executing the recipe multiple times,
# 	   check for the build stage, only running it on stage1.
define compile_devicetree
	$(if $(call is_prestage,stage1), \
		$(QBUILD)$(dtc) --format=$(1) --sections=$(2) -o $@.tmp $< && \
		diff $@ $@.tmp 1>/dev/null 2>&1 || { echo "updating $@"; cp $@.tmp $@; } \
		, \
	)
endef

$(devtree_preproc): $(CONFIG_DEVICE_TREE)
	$(call preproc_file)

$(devtree_src): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,c,all)

$(devtree_make): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,make,memory$(comma)arch)

$(devtree_header): $(devtree_preproc) $(dtc)
	$(call compile_devicetree,header,memory$(comma)arch)


$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.o,          $(devtree_src),)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.host.o,     $(devtree_src), host)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.libbrick.o, $(devtree_src),)


####
## device tree memory linker script
####

lds_memory := $(build_tree)/scripts/linker/gen_lds_memory
lds_memory_script := $(loc_build_tree)/memory.lds

$(lds_memory_script): $(lds_memory)
	$(call compile_file,$(lds_memory) $(lds_memory_script))

prepare_deps: $(lds_memory_script)


## kernel flags
ldlibs-kernel += \
	-T$(lds_memory_script)

## init application flags
ldlibs-app += \
	-T$(lds_memory_script)
