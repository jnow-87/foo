#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := hardware.o atomic.o opts.o syscall.o kernel/ linux/
obj-$(CONFIG_CODE_COVERAGE) += gcov/ gcov/libgcov.a

# libsys
obj-nobuiltin-y := libsys.o

libsys-y := hardware.libsys.o atomic.libsys.o opts.libsys.o syscall.libsys.o lib/ $(loc_build_tree)/linux/libsys.o
libsys-$(CONFIG_CODE_COVERAGE) += $(loc_build_tree)/gcov/libsys.o $(loc_build_tree)/gcov/libgcov.a


# custom rules to generate separate object files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libsys.o,    $(loc_src_tree)/%.c,)


####
## flags
####

ifeq ($(CONFIG_CODE_COVERAGE),y)
  ldlibs-kernel += -L$(loc_build_tree)/gcov
  ldlibs-app += -L$(loc_build_tree)/gcov
endif
