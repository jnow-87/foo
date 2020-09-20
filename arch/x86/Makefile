#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



# kernel
obj-y := start.o memory.o

# libsys
obj-nobuiltin-y := libsys.o

libsys-y := lib.o


# custom rules to generate separate object files for src files that are reuired by kernel and libsys
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.libsys.o,    $(loc_src_tree)/%.c,)
