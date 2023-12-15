#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := init.elf

init-y := main.o shell/

linker_script := $(build_tree)/scripts/linker/app_plugin.i.lds

# ensure a rebuild if non-object-file dependencies are updated
init-y += \
	$(loc_src_tree)/plugin.lds \
	$(linker_script) \
	$(build_tree)/lib/libbrick.a


####
# flags
####

init-cppflags-y := \
	-Iinclude/lib \
	-I$(loc_src_tree)/include

init-ldlibs-y += \
	$(ldlibs-app) \
	-L$(loc_src_tree) \
	-T$(linker_script) \
	-lbrick \
	-lgcc
