#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := system.o 

obj-$(CONFIG_FONT_SYSTEM_VERT) += system_vert.o


####
## rules
####

fontc := $(build_tree)/scripts/fontc/fontc

# horizontal fonts
$(loc_build_tree)/%.c: $(loc_src_tree)/%.font $(fontc)
	$(call compile_file,$(fontc) --output-file $@ $<)

# vertical fonts
$(loc_build_tree)/%_vert.c: $(loc_src_tree)/%.font $(fontc)
	$(call compile_file,$(fontc) --vertical --output-file $@ $<)

# common build
$(call gen_rule_basic, compile_o_c, $(loc_build_tree)/%.o, $(loc_build_tree)/%.c)
