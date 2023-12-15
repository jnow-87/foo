#
# Copyright (C) 2020 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := gen_lds_memory

gen_lds_memory-y := lds_memory.o sys/


hostcppflags-y := -Iinclude
hostcflags-y := \
	-Wno-unused-function \
	-fno-builtin


####
## preprocess linker scripts
####

$(loc_build_tree)/%.i.lds: $(loc_src_tree)/%_$(CONFIG_ARCH).lds
	$(call preproc_file)

$(loc_build_tree)/%_plugin.i.lds: cppflags := -DBUILD_USER_PLUGIN
$(loc_build_tree)/%_plugin.i.lds: $(loc_src_tree)/%_$(CONFIG_ARCH).lds
	$(call preproc_file)
