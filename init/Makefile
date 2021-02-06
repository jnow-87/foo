#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := init.elf


init-y := main.o shell/ scripts/linker/app_$(CONFIG_ARCH).lds

# ensure init is rebuild if libsys is updated
init-y += $(build_tree)/lib/libsys.a


####
# flags
####

init-cppflags-y := \
	-I"include/lib" \
	-I$(loc_src_tree)

init-ldlibs-y += \
	$(ldlibs-app) \
	-T$(loc_src_tree)/lists.lds \
	-Tapp_$(CONFIG_ARCH).lds \
	-lsys \
	-lgcc
