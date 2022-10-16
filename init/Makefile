#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := init.elf


init-y := main.o shell/

# ensure a rebuild if indirect dependencies are updated
init-y += \
	scripts/linker/app_$(CONFIG_ARCH).lds \
	$(loc_src_tree)/lists.lds \
	$(build_tree)/lib/libbrick.a


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
	-lbrick \
	-lgcc
