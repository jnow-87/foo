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

kernel_lds_deps := \
	kernel_init.i.lds \
	kernel_stat.i.lds \
	kernel_test.i.lds \
	std_init.i.lds \
	fonts.i.lds \
	discard.i.lds

app_lds_deps := \
	libsys_init.i.lds \
	std_init.i.lds \
	fonts.i.lds \
	discard.i.lds

$(loc_build_tree)/kernel.i.lds: $(addprefix $(loc_build_tree)/, $(kernel_lds_deps))
$(loc_build_tree)/app.i.lds: $(addprefix $(loc_build_tree)/, $(app_lds_deps))
$(loc_build_tree)/app_plugin.i.lds: $(addprefix $(loc_build_tree)/, $(app_lds_deps))

$(loc_build_tree)/%.i.lds: $(loc_src_tree)/%.lds
	$(call preproc_file)

$(loc_build_tree)/%.i.lds: $(loc_src_tree)/%_$(CONFIG_ARCH).lds
	$(call preproc_file)

# linker scripts that contain a hook for a plugin linker script
$(loc_build_tree)/%_plugin.i.lds: cppflags += -DBUILD_USER_PLUGIN
$(loc_build_tree)/%_plugin.i.lds: $(loc_src_tree)/%_$(CONFIG_ARCH).lds
	$(call preproc_file)


####
## memory linker script
####

$(loc_build_tree)/memory.lds: $(loc_build_tree)/gen_lds_memory
	$(call compile_file,$< $@)
