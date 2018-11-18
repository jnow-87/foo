#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := init.elf


init-y := main.o shell/

# ensure init is rebuild if libsys is updated
init-y += $(build_tree)/lib/$(lib_name)


# additional init linker options
init-cppflags := -I"include/lib" -I$(loc_src_tree) -I$(loc_build_tree)
init-ldlibs := \
	$(ldlibs-app-arch) \
	-T$(loc_src_tree)/lists.lds \
	-lsys \
	-lgcc

# init binary name required by main Makefile
init := $(loc_build_tree)/init.elf
