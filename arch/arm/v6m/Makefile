#
# Copyright (C) 2023 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := \
	init.o \
	syscall.o \
	kernel/

# libbrick
obj-nobuiltin-y := libbrick.o

libbrick-y := \
	init.o \
	syscall.libbrick.o \
	lib/


# custom rules to generate separate object files for src files that are required by kernel and libbrick
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libbrick.o,    $(loc_src_tree)/%.c,)
