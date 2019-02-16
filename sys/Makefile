#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := errno.o string.o memblock.o mutex.o list1.o list2.o vector.o

obj-$(CONFIG_KERNEL_PRINTF) += stream.o
obj-$(CONFIG_SYS_RINGBUFFER) += ringbuf.o

hostobj-y := errno.o string.o stream.o memblock.o ringbuf.o list1.o list2.o vector.o

obj-nobuiltin-y := libsys.o


libsys-y := errno.o string.o list1.o list2.o vector_lib.o

libsys-$(CONFIG_SC_DYNMEM) += memblock.o
libsys-$(CONFIG_LIB_MUTEX) += mutex_lib.o
libsys-$(CONFIG_LIB_STREAM) += stream_lib.o


# custom rules to generate separate objects files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%_lib.o,    $(loc_src_tree)/%.c,)
