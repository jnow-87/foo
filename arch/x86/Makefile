#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := hardware.o atomic.o opts.o syscall.o kernel/ linux/
obj-$(CONFIG_CODE_COVERAGE) += gcov/ gcov/libgcov.a

subdir-y := emu/

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := hardware.libbrick.o atomic.libbrick.o opts.libbrick.o syscall.libbrick.o lib/ $(loc_build_tree)/linux/libbrick.o
libbrick-$(CONFIG_CODE_COVERAGE) += $(loc_build_tree)/gcov/libbrick.o $(loc_build_tree)/gcov/libgcov.a


# custom rules to generate separate object files for src files that are required by kernel and libbrick
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libbrick.o,    $(loc_src_tree)/%.c,)


####
## flags
####

ldflags += -Wl,-znoexecstack
ldlibs += -Wl,-znoexecstack

ifeq ($(CONFIG_CODE_COVERAGE),y)
  ldlibs-kernel += -L$(loc_build_tree)/gcov
  ldlibs-app += -L$(loc_build_tree)/gcov
endif
