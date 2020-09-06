#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := errno.o string.o memblock.o mutex.o list1.o list2.o vector.o devtree.o devicetree.o

obj-$(CONFIG_KERNEL_PRINTF) += stream.o
obj-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o
obj-$(CONFIG_SYS_PATTERN_MATCH) += patmat.o
obj-$(CONFIG_KERNEL_NET) += inet.o

# host
hostobj-y := errno.o string.o stream.o memblock.o memory.o ringbuf.o list1.o list2.o vector.o devtree.o devicetree.o patmat.o inet.o

hostcflags-y += \
	-fno-builtin \
	-fprofile-arcs \
	-ftest-coverage \
	--coverage

# libsys
obj-nobuiltin-y := libsys.o

libsys-y := errno.libsys.o string.libsys.o list1.libsys.o list2.libsys.o vector.libsys.o

libsys-$(CONFIG_SC_DYNMEM) += memblock.libsys.o
libsys-$(CONFIG_LIB_MUTEX) += mutex.libsys.o
libsys-$(CONFIG_LIB_STREAM) += stream.libsys.o
libsys-$(CONFIG_SYS_RINGBUFFER) += ringbuf.libsys.o
libsys-$(CONFIG_SYS_PATTERN_MATCH) += patmat.libsys.o
libsys-$(CONFIG_KERNEL_NET) += inet.libsys.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libsys.o,    $(loc_src_tree)/%.c,)


####
## device tree
####

dtc := $(build_tree)/scripts/devtree/dtc
devtree_make := $(loc_build_tree)/devicetree.make
devtree_header := $(loc_build_tree)/devicetree.h
devtree_src := $(loc_build_tree)/devicetree.c

prepare_deps: $(devtree_header) $(devtree_make)

$(devtree_src): $(CONFIG_DEVICE_TREE) $(dtc)
	$(call compile_file,$(dtc) --format=c --sections=all -o $@ $<)

$(devtree_header): $(CONFIG_DEVICE_TREE) $(dtc)
	$(call compile_file,$(dtc) --format=header --sections=memory -o $@ $<)

$(devtree_make): $(CONFIG_DEVICE_TREE) $(dtc)
	$(call compile_file,$(dtc) --format=make --sections=memory -o $@ $<)


$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.o,      $(loc_build_tree)/devicetree.c,)
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/devicetree.host.o, $(loc_build_tree)/devicetree.c, host)


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
