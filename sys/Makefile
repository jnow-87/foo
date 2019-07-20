#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := errno.o string.o memblock.o mutex.o list1.o list2.o vector.o devtree.o devicetree.o

obj-$(CONFIG_KERNEL_PRINTF) += stream.o
obj-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o

hostobj-y := errno.o string.o stream.o memblock.o ringbuf.o list1.o list2.o vector.o devtree.o devicetree.o

obj-nobuiltin-y := libsys.o


libsys-y := errno.o string.o list1.o list2.o vector_lib.o

libsys-$(CONFIG_SC_DYNMEM) += memblock.o
libsys-$(CONFIG_LIB_MUTEX) += mutex_lib.o
libsys-$(CONFIG_LIB_STREAM) += stream_lib.o
libsys-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)


####
## device tree
####

devtree := $(build_tree)/scripts/devtree/devtree

prepare_deps: $(loc_build_tree)/devicetree.make $(loc_build_tree)/devicetree.h


$(loc_build_tree)/devicetree.c: $(CONFIG_DEVICE_TREE) $(devtree)
	$(mkdir) $(dir $@)
	$(QBUILD)$(devtree) --format=c --sections=all -o $@ $<

$(loc_build_tree)/devicetree.h: $(CONFIG_DEVICE_TREE) $(devtree)
	$(mkdir) $(dir $@)
	$(QBUILD)$(devtree) --format=header --sections=memory -o $@ $<

$(loc_build_tree)/devicetree.make: $(CONFIG_DEVICE_TREE) $(devtree)
	$(mkdir) $(dir $@)
	$(QBUILD)$(devtree) --format=make --sections=memory -o $@ $<


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
